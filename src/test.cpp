#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <typeinfo>

#include "../include/unique_ptr.hpp"
#include "../include/Value.hpp"


#define P_TYPE(x) (typeToChar(x))

using namespace TOML; // yeah yeah I know, it's testing, gimme a break

inline void	newtest(void)
{
	static int	i(1);

	std::cout << "\e[0;30;47m===============TEST " << i++ << "===============\e[0m"
		<< std::endl;
}

class Base
{
	public:
		virtual Value::e_type	getType() const { return Value::T_UNDEF; }
		bool	isInt() const { return this->getType() == Value::T_INT; }
		virtual ~Base() {};

		virtual __int64_t&			getInt()		{ throw(std::logic_error("wtf")); }
		virtual __int64_t const&	getInt() const  { throw(std::logic_error("wtf")); }
};

class String : public Base
{
	public:
		typedef typename	std::string		value_type;

	private:
		value_type	_val;
	
	public:
		Value::e_type	getType() const { return Value::T_STRING; }
		value_type&	getStr() {return _val; }
		// value_type const&	getStr() const { return _val; }

};

class Integer : public Base
{
	public:
		typedef 	__int64_t		value_type;

	private:
		value_type	_val;
	
	public:
		Integer(value_type x) : _val(x) {}
		Value::e_type	getType() const { return Value::T_INT; }
		value_type&			getInt()		{ return _val; }
		value_type const&	getInt() const  { return _val; }

};

const char* typeToChar(Value::e_type type)
{
	switch (type)
	{
		case Value::T_INT:
			return ("Int");
		case Value::T_FLOAT:
			return ("Float");
		case Value::T_BOOL:
			return ("Bool");
		case Value::T_STRING:
			return ("String");
		case Value::T_GROUP:
			return ("Group");
		case Value::T_UNDEF:
			return ("Undefined");
		default:
			return ("unknown type");
	}
}

int	main(void)
{
	newtest();
	{
		Base* base1 = static_cast<Base*>(new Integer(42));
		Base* base2 = static_cast<Base*>(new String);
		std::cout << P_TYPE(base1->getType()) << std::endl;
		std::cout << P_TYPE(base2->getType()) << std::endl;
		delete base1;
		delete base2;
	}
	newtest();
	{
		std::vector<Base*>	vecbase;
		vecbase.push_back(new Integer(42));
		vecbase.push_back(new String);
		vecbase.push_back(new Integer(42));

		for (size_t i = 0; i < vecbase.size(); ++i) {
			std::cout << P_TYPE(vecbase[i]->getType()) << std::endl;
			delete vecbase[i];
		}
	}
	newtest();
	try
	{
		std::vector<Base>	vecbase;
		vecbase.push_back(Integer(42));
		vecbase.push_back(String());

		Base*	test = &vecbase[0];
		std::cout << P_TYPE(test->getType()) << std::endl;
		Integer* test2 = static_cast<Integer*>(&vecbase[0]);
		std::cout << P_TYPE(test2->getType()) << std::endl;
		Integer &test3 = dynamic_cast<Integer&>(vecbase[0]);
		std::cout << P_TYPE(test3.getType()) << std::endl;
	}
	catch (std::exception const& e)
	{
		std::cout << "Caught exception " << e.what() << std::endl;
	}
	newtest();
	{
		Base* base1 = static_cast<Base*>(new Integer(42));
		Base* base2 = static_cast<Base*>(new String);
		std::cout << P_TYPE(base1->getType()) << std::endl;
		std::cout << "IsInt(): " << base1->isInt() << std::endl;
		std::cout << P_TYPE(base2->getType()) << std::endl;
		std::cout << "IsInt(): " << base2->isInt() << std::endl;
		delete base1;
		delete base2;
	}
	newtest();
	{
		Base* base1 = static_cast<Base*>(new Integer(42));
		std::cout << P_TYPE(base1->getType()) << std::endl;
		std::cout << "IsInt(): " << base1->isInt() << std::endl;
		std::cout << "value: " << base1->getInt() << std::endl;
		delete base1;
	}
	newtest();
	{
		std::cout << "Size of value: " << sizeof(Value) << std::endl; // rho putain
		std::cout << "Size of string: " << sizeof(std::string) << std::endl;
	}
	newtest();
	{
		std::vector<Value>	vec;
		vec.push_back(Value("group"));
		vec.push_back(Value("test", 42, Value::T_INT));
		vec.push_back(Value("test", 42, Value::T_FLOAT));
		vec.push_back(Value("test", false, Value::T_BOOL));

		for (size_t i = 0; i < vec.size(); ++i) {
			if (vec[i].isFundamental()) {
				vec[i].set(21);
			}
			std::cout << i << " - " << "Type: " << P_TYPE(vec[i].type())
				<< " | " << "Value: " << vec[i] << std::endl;
		}
	}
	return 0;
}
