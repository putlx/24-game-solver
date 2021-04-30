#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

using id_type = std::variant<struct operation_set, double>;

struct operation {
    std::shared_ptr<id_type> operand;
    bool com;
    std::size_t cnt;
};

bool operator==(const operation &, const operation &);

struct operation_set {
    bool prioritized;
    std::list<operation> set;

    void update(const operation_set &r, bool rvs) {
        for (const auto &a : r.set)
            insert(operation{a.operand, a.com != rvs, 1});
    }

    void insert(const operation &i) {
        for (auto &a : set) {
            if (a == i) {
                a.cnt += i.cnt;
                return;
            }
        }
        set.push_front(i);
    }
};

std::ostream &operator<<(std::ostream &out, const id_type &i) {
    if (std::holds_alternative<double>(i)) {
        auto v = std::get<double>(i);
        if (v < 0)
            return out << '(' << v << ')';
        return out << v;
    }
    const auto &s = std::get<operation_set>(i);
    out << '(' << s.prioritized;
    for (const auto &a : s.set)
        for (std::size_t i = 0; i < a.cnt; ++i)
            out << (s.prioritized ? (a.com ? '*' : '/') : (a.com ? '+' : '-')) << *a.operand;
    return out << ')';
}

bool operator==(const operation_set &lhs, const operation_set &rhs) {
    return lhs.prioritized == rhs.prioritized && lhs.set.size() == rhs.set.size() &&
           std::all_of(lhs.set.cbegin(), lhs.set.cend(), [&](const auto &a) {
               return std::any_of(rhs.set.cbegin(), rhs.set.cend(), [&](const auto &b) { return a == b; });
           });
}

bool operator==(const operation &lhs, const operation &rhs) {
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
        auto &expr = exprs.front();
        if (std::fabs(expr.value - X) < 0.01 &&
            std::none_of(ids.cbegin(), ids.cend(), [&](const auto &id) { return *id == *expr.id; })) {
            ids.push_front(expr.id);
            std::cout << expr << '=' << X << std::endl;
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

                expr gexpr{&exprs[x], &exprs[y], &oprs[k], oprs[k].fn(exprs[x].value, exprs[y].value),
                           std::make_shared<id_type>(std::in_place_type<operation_set>)};

                auto &id = std::get<operation_set>(*gexpr.id);
                auto &lid = (gexpr.op->reverse ? gexpr.rhs : gexpr.lhs)->id;
                auto &rid = (gexpr.op->reverse ? gexpr.lhs : gexpr.rhs)->id;
                id.prioritized = gexpr.op->priority > 0;
                if (std::holds_alternative<double>(*lid) ||
                    std::get<operation_set>(*lid).prioritized != id.prioritized)
                    id.insert(operation{lid, true, 1});
                else
                    id.update(std::get<operation_set>(*lid), false);
                if (std::holds_alternative<double>(*rid) ||
                    std::get<operation_set>(*rid).prioritized != id.prioritized)
                    id.insert(operation{rid, gexpr.op->ch == '+' || gexpr.op->ch == '*', 1});
                else
                    id.update(std::get<operation_set>(*rid), gexpr.op->ch == '-' || gexpr.op->ch == '/');

                std::vector<expr> gexprs(exprs.size() - 1);
                gexprs[0] = std::move(gexpr);
                for (std::size_t z = 0, i = 1; z < exprs.size(); ++z)
                    if (z != x && z != y)
                        gexprs[i++] = exprs[z];
                solve(std::move(gexprs));
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
        solve(std::move(exprs));
        std::cout << '[' << ids.size() << " solution" << (ids.size() < 2 ? "" : "s") << "]\n\n";
        ids.clear();
    }
    return 0;
}
