#include "assert.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <streambuf>
#include <string_view>
#include <string>

using namespace std::literals;

void test_path_differentiation();

void custom_fail(assert_detail::assertion_printer& printer, assert_detail::assert_type, assert_detail::ASSERTION) {
	std::cout<<assert_detail::strip_colors(printer(0))<<std::endl<<std::endl;
}

template<typename T>
struct printable {
	std::optional<T> f;
	printable(T t) : f(t) {}
	bool operator==(const printable& other) const {
		return f == other.f;
	}
};
template<typename T>
struct not_printable {
	std::optional<T> f;
	not_printable(T t) : f(t) {}
	bool operator==(const not_printable& other) const {
		return f == other.f;
	}
};
template<typename T>
std::ostream& operator<<(std::ostream& stream, const printable<T>& p) {
	return stream<<"(printable = "<<*p.f<<")";
}

int foo() {
	std::cout<<"foo() called"<<std::endl;
	return 2;
}
int bar() {
	std::cout<<"bar() called"<<std::endl;
	return -2;
}

struct logger_type {
	int n;
	logger_type(int n): n(n) {
		std::cout<<"logger_type::logger_type() [n="<<n<<"]"<<std::endl;
	}
	compl logger_type() {
		std::cout<<"logger_type::compl logger_type() [n="<<n<<"]"<<std::endl;
		n = -1;
	}
	logger_type(const logger_type& other): n(other.n) {
		std::cout<<"logger_type::logger_type(const logger_type&) [n="<<n<<"]"<<std::endl;
	}
	logger_type(logger_type&& other): n(other.n) {
		other.n = -2;
		std::cout<<"logger_type::logger_type(logger_type&&) [n="<<n<<"]"<<std::endl;
	}
	logger_type& operator=(const logger_type& other) {
		n = other.n;
		std::cout<<"logger_type::operator=(const logger_type&) [n="<<n<<"]"<<std::endl;
		return *this;
	}
	logger_type& operator=(logger_type&& other) {
		n = other.n;
		other.n = -2;
		std::cout<<"logger_type::operator=(logger_type&&) [n="<<n<<"]"<<std::endl;
		return *this;
	}
	bool operator==(const logger_type& other) const {
		std::cout<<"logger_type::operator==(const logger_type&) [n="<<n<<", other="<<other.n<<"]"<<std::endl;
		return false;
	}
};
bool operator==(const logger_type& a, int b) {
	std::cout<<"logger_type::operator==(const logger_type&, int) [n="<<a.n<<", b="<<b<<"]"<<std::endl;
	return false;
}
bool operator==(int a, const logger_type& b) {
	std::cout<<"logger_type::operator==(int, const logger_type&) [b="<<a<<", n="<<b.n<<"]"<<std::endl;
	return false;
}
std::ostream& operator<<(std::ostream& stream, const logger_type& lt) {
	return stream<<"logger_type [n = "<<lt.n<<"]";
}

#line 500
void rec(int n) {
	if(n == 0) assert(false);
	else rec(n - 1);
}

void recursive_a(int), recursive_b(int);

void recursive_a(int n) {
	if(n == 0) assert(false);
	else recursive_b(n - 1);
}

void recursive_b(int n) {
	if(n == 0) assert(false);
	else recursive_a(n - 1);
}

#define SECTION(s) std::cout<<"===================== ["<<s<<"] ====================="<<std::endl;

// TODO: need to check assert, verify, and check...?
// Opt/DNDEBUG

#line 1000
template<typename T>
class test_class {
public:
	template<typename S> void something([[maybe_unused]] std::pair<S, T> x) {
		something_else();
	}

	void something_else() {
		// FIXME: Check all stack traces on msvc... Bug with __PRETTY_FUNCTION__ in lambdas.
		// value printing: strings
		SECTION("value printing: strings");
		#line 1100
		{
			std::string s = "test\n";
			int i = 0;
			assert(s == "test");
			assert(s[i] == 'c', "", s, i);
			char* buffer = nullptr;
			char thing[] = "foo";
			assert(buffer == thing);
			assert(buffer == +thing);
			std::string_view sv = "foo";
			assert(s == sv);
		}
		// value printing: pointers
		SECTION("value printing: pointers");
		#line 1200
		{
			assert((uintptr_t)-1 == 0xff);
			assert((uintptr_t)-1 == (uintptr_t)0xff);
			void* foo = (void*)0xdeadbeef;
			assert(foo == nullptr);
		}
		// value printing: number formats
		SECTION("value printing: number formats");
		#line 1300
		{
			const uint16_t flags = 0b000101010;
			const uint16_t mask = 0b110010101;
			assert(mask bitand flags);
			assert(0xf == 16);
		}
		// value printing: floating point
		SECTION("value printing: floating point");
		#line 1400
		{
			assert(1 == 1.5);
			assert(0.5 != .5); // FIXME
			assert(0.1 + 0.2 == 0.3);
			VERIFY(.1 + .2 == .3);
			assert(0.1f + 0.2f == 0.3f);
			float ff = .1f;
			assert(ff == .1);
			assert(.1f == .1);
		}
		// value printing: ostream overloads
		SECTION("value printing: ostream overloads");
		#line 1500
		{
			printable p{1.42};
			assert(p == printable{2.55});
		}
		// value printing: no ostream overload
		SECTION("value printing: no ostream overload");
		#line 1600
		{
			const not_printable p{1.42};
			assert(p == not_printable{2.55});
			assert(p.f == not_printable{2.55}.f);
		}

		// optional messages
		SECTION("optional messages");
		#line 1700
		{
			assert(false, 2);
			assert(false, "foo");
			assert(false, "foo"s);
			assert(false, "foo"sv);
			assert(false, (char*)"foo");
			assert(false, "foo", 2);
			assert(false, "foo"s, 2);
			assert(false, "foo"sv, 2);
			assert(false, (char*)"foo", 2);
		}
		// extra diagnostics
		SECTION("errno");
		#line 1800
		{
			errno = 2;
			assert(false, errno);
			errno = 2;
			assert(false, "foo", errno);
		}
		// general
		SECTION("general");
		#line 1900
		{
			assert(false, "foo", false, 2 * foo(), "foobar"sv, bar(), printable{2.55});
		}
		// safe comparisons
		SECTION("safe comparisons");
		#line 2000
		{
			assert(18446744073709551606ULL == -10);
			assert(-1 > 1U);
		}

		// expression decomposition
		SECTION("expression decomposition");
		#line 2100
		{
			//ASSERT(1 = (1 bitand 2)); // <- FIXME: Not evaluated correctly
			assert(1 == (1 bitand 2));
			assert(1 < 1 < 0);
			assert(0 + 0 + 0);
			assert(false == false == false);
			assert(1 << 1 == 200);
			assert(1 << 1 << 31);
			int x = 2;
			assert(x -= 2);
			x = 2;
			assert(x -= x -= 1); // TODO: double check....
			x = 2;
			assert(x -= x -= x -= 1); // TODO: double check....
			assert(true ? false : true, "pffft");
		}
		// ensure values are only computed once
		SECTION("ensure values are only computed once");
		#line 2200
		{
			assert(foo() < bar());
		}
		// value forwarding: copy / moves
		SECTION("value forwarding: copy / moves");
		#line 2300
		{
			{
				logger_type lt1(1);
				logger_type lt2(2);
				assert(lt1 == lt2);
			}
			std::cout<<"--------------------------------------------"<<std::endl<<std::endl;
			{
				assert(1 == logger_type(2));
			}
			std::cout<<"--------------------------------------------"<<std::endl<<std::endl;
			{
				assert(logger_type(1) == 2);
			}
			std::cout<<"--------------------------------------------"<<std::endl<<std::endl;
			{
				auto r = assert(logger_type(1) == logger_type(2));
				VERIFY(!(std::is_same<decltype(r), logger_type>::value));
				VERIFY(r.n != 1);
			}
			std::cout<<"--------------------------------------------"<<std::endl<<std::endl;
		}
		// value forwarding: lifetimes
		// TODO
		// value forwarding: lvalue references
		SECTION("value forwarding: lvalue references");
		#line 2400
		{
			{
				logger_type lt(2);
				decltype(auto) x = get_lt_a(lt);
				x.n++;
				assert(lt == 1);
				assert(x == 1);
			}
			std::cout<<"--------------------------------------------"<<std::endl<<std::endl;
			{
				int x = 1;
				assert(x ^= 1);
				//assert(x ^= 1) |= 0xf0; // FIXME
				//assert(x == 0);
			}
		}
		// value forwarding: rvalues
		SECTION("value forwarding: rvalues");
		#line 2500
		{
			{
				auto x = get_lt_b();
				assert(false, x);
			}
			std::cout<<"--------------------------------------------"<<std::endl<<std::endl;
		}
		
		// general stack traces are tested throughout
		// simple recursion
		SECTION("simple recursion");
		#line 2600
		rec(10); // FIXME: Check stacktrace in clang
		// other recursion
		SECTION("other recursion");
		#line 2700
		recursive_a(10); // FIXME: Check stacktrace in clang

		// path differentiation
		SECTION("Path differentiation");
		#line 2800
		test_path_differentiation();
	}

	decltype(auto) get_lt_a(logger_type& l) {
		return assert(l == 2);
	}

	decltype(auto) get_lt_b() {
		return assert(logger_type(2) == 2);
	}
};

struct N { };

#line 400
int main() {
	test_class<int> t;
	t.something(std::pair {N(), 1});
}
