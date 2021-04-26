#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

struct oprn_set;

using id_type = std::variant<oprn_set, double>;

struct oprn {
    std::shared_ptr<id_type> operand;
    bool com;
    std::size_t cnt;
};

bool operator==(const oprn &, const oprn &);

struct oprn_set {
    bool prioritized;
    std::list<std::shared_ptr<oprn>> set;

    void insert(const std::shared_ptr<id_type> &a, bool com) {
        insert(std::shared_ptr<oprn>(new oprn{a, com, 1}));
    }

    void update(const oprn_set &other, bool rvs) {
        for (const auto &m : other.set) {
            std::shared_ptr<oprn> a(new oprn(*m));
            if (rvs)
                a->com = !a->com;
            insert(std::move(a));
        }
    }

  private:
    void insert(const std::shared_ptr<oprn> &a) {
        for (auto &m : set)
            if (*m == *a) {
                ++m->cnt;
                return;
            }
        set.push_front(a);
    }
};

std::ostream &operator<<(std::ostream &out, const id_type &a) {
    if (std::get_if<double>(&a)) {
        if (std::get<double>(a) < 0)
            return out << '(' << std::get<double>(a) << ')';
        return out << std::get<double>(a);
    }
    const auto &m = std::get<oprn_set>(a);
    out << '(' << (m.prioritized ? '1' : '0');
    for (const auto &n : m.set)
        for (std::size_t i = 0; i < n->cnt; ++i)
            out << (m.prioritized ? (n->com ? '*' : '/') : (n->com ? '+' : '-')) << *n->operand;
    return out << ')';
}

bool operator==(const oprn_set &lhs, const oprn_set &rhs) {
    if (lhs.prioritized != rhs.prioritized)
        return false;
    else if (lhs.set.size() != rhs.set.size())
        return false;
    return std::all_of(lhs.set.cbegin(), lhs.set.cend(), [&](const auto &a) {
        return std::any_of(rhs.set.cbegin(), rhs.set.cend(), [&](const auto &b) { return *a == *b; });
    });
}

bool operator==(const oprn &lhs, const oprn &rhs) {
    return lhs.com == rhs.com && lhs.cnt == rhs.cnt && *lhs.operand == *rhs.operand;
}

struct opr {
    double (*const fn)(double, double);
    const char ch;
    const int priority;
    const bool reverse;
};

struct expr {
    const expr *lhs;
    const expr *rhs;
    const opr *op;
    double value;
    std::shared_ptr<id_type> id;
};

std::ostream &operator<<(std::ostream &out, const expr &e) {
    if (!e.op) {
        if (e.value < 0)
            return out << '(' << e.value << ')';
        return out << e.value;
    }

    const expr *lhs, *rhs;
    if (e.op->reverse) {
        lhs = e.rhs;
        rhs = e.lhs;
    } else {
        lhs = e.lhs;
        rhs = e.rhs;
    }

    if (lhs->op && lhs->op->priority < e.op->priority)
        out << '(' << *lhs << ')';
    else
        out << *lhs;
    out << e.op->ch;
    if (rhs->op && rhs->op->priority < e.op->priority)
        out << '(' << *rhs << ')';
    else
        out << *rhs;
    return out;
}

static int N = 4;
static double X = 24.;
static std::list<std::shared_ptr<id_type>> ids;

void solve(const std::vector<expr> &exprs) {
    static const opr oprs[] = {{[](double x, double y) { return x + y; }, '+', 0, false},
                               {[](double x, double y) { return x - y; }, '-', 0, false},
                               {[](double x, double y) { return x * y; }, '*', 1, false},
                               {[](double x, double y) { return x / y; }, '/', 1, false},
                               {[](double x, double y) { return y - x; }, '-', 0, true},
                               {[](double x, double y) { return y / x; }, '/', 1, true}};

    if (exprs.size() == 1) {
        auto &exp = exprs.front();
        if (std::fabs(exp.value - X) < 0.01 &&
            std::none_of(ids.cbegin(), ids.cend(), [&](const auto &id) { return *id == *exp.id; })) {
            ids.push_front(exp.id);
            std::cout << exp << '=' << X << std::endl;
        }
        return;
    }

    for (std::size_t x = 0; x < exprs.size(); ++x) {
        for (std::size_t y = x + 1; y < exprs.size(); ++y) {
            for (int k = 0; k < 6; ++k) {
                if (oprs[k].reverse && exprs[x].op && exprs[x].op->priority == oprs[k].priority)
                    continue;
                else if (!oprs[k].reverse && exprs[y].op && exprs[y].op->priority == oprs[k].priority)
                    continue;

                std::vector<expr> gexprs(exprs.size() - 1);
                auto &gexpr = gexprs.front();
                gexpr.lhs = &exprs[x];
                gexpr.rhs = &exprs[y];
                gexpr.op = &oprs[k];
                gexpr.value = oprs[k].fn(exprs[x].value, exprs[y].value);
                gexpr.id = std::make_shared<id_type>(std::in_place_type<oprn_set>);

                auto &id = std::get<oprn_set>(*gexpr.id);
                auto &lid = (gexpr.op->reverse ? gexpr.rhs : gexpr.lhs)->id;
                auto &rid = (gexpr.op->reverse ? gexpr.lhs : gexpr.rhs)->id;
                id.prioritized = gexpr.op->priority > 0;

                if (std::get_if<double>(&*lid) || std::get<oprn_set>(*lid).prioritized != id.prioritized)
                    id.insert(lid, true);
                else
                    id.update(std::get<oprn_set>(*lid), false);

                if (std::get_if<double>(&*rid) || std::get<oprn_set>(*rid).prioritized != id.prioritized)
                    id.insert(rid, gexpr.op->ch == '+' || gexpr.op->ch == '*');
                else
                    id.update(std::get<oprn_set>(*rid), gexpr.op->ch == '-' || gexpr.op->ch == '/');

                for (std::size_t z = 0, i = 1; z < exprs.size(); ++z)
                    if (z != x && z != y)
                        gexprs[i++] = exprs[z];
                solve(gexprs);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::string_view("-h") == argv[i] || std::string_view("--help") == argv[i]) {
            std::cout << "Usage: " << argv[0] << " [-h, --help] [N] [X]\n"
                      << "Arguments:\n"
                      << "  N: number of operands involved (default 4)\n"
                      << "  X: target value (default 24)\n";
            return 0;
        }
    }
    if (argc > 1 && (N = std::atoi(argv[1])) <= 0)
        N = 4;
    if (argc > 2)
        X = std::round(std::atof(argv[2]));
    std::cout << "Give " << N << " integer" << (N < 2 ? "" : "s") << " to work out " << X << ":\n\n";

    while (true) {
    begin:;
        std::vector<expr> exprs(N);
        for (auto &exp : exprs) {
            if (!(std::cin >> exp.value)) {
                std::cin.clear();
                while (std::cin.get() != '\n')
                    if (std::cin.eof())
                        return 0;
                std::cout << "[invalid input]\n\n";
                goto begin;
            }
            exp.value = std::round(exp.value);
            exp.op = nullptr;
            exp.id = std::make_shared<id_type>(std::in_place_type<double>, exp.value);
        }
        solve(exprs);
        std::cout << '[' << ids.size() << " solution" << (ids.size() < 2 ? "" : "s") << "]\n\n";
        ids.clear();
    }
    return 0;
}
