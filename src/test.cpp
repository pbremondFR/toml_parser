#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <typeinfo>

// #include "../include/unique_ptr.hpp"
#include "../include/Value.hpp"
#include "../include/Document.hpp"

#ifndef P_TYPE
# define P_TYPE(x) (TOML::typeToChar(x))
#endif

using namespace TOML; // yeah yeah I know, it's testing, gimme a break

static std::string	centeredString(const std::string& src, std::size_t len)
{
	std::ptrdiff_t	pad_len = len - src.length();
	if (pad_len < 0)
		pad_len = 0;
	std::string	out(pad_len / 2, ' ');

	out += src;
	out.append(pad_len / 2 + pad_len % 2, ' ');
	return (out);
}

inline void	newtest(const char *description = NULL)
{
	static int	i = 1;
	std::stringstream	header;
	header << "===============TEST " << i++ << "===============";

	std::cout << "\e[0;30;47m" << header.str() << "\e[0m" << std::endl;
	if (description)
	{
		std::cout << "\e[0;30;47m" << centeredString(description, header.str().length())
		<< "\e[0m" << std::endl;
	}
}

int	main(int argc, const char *argv[])
{
	if (argc != 2) {
		std::cerr << "Bruh" << std::endl;
		return 1;
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
		vec.push_back(Value("test", 42,		Value::T_INT));
		vec.push_back(Value("test", 42,		Value::T_FLOAT));
		vec.push_back(Value("test", false,	Value::T_BOOL));

		for (size_t i = 0; i < vec.size(); ++i) {
			if (vec[i].isFundamental()) {
				vec[i].set(21);
			}
			std::cout << i << " - " << "Type: " << P_TYPE(vec[i].type())
				<< " | " << "Value: " << vec[i] << std::endl;
		}
	}
	newtest();
	try
	{
		Document	doc(argv[1]);
		doc.parse();

		std::cout << P_TYPE( doc.at("rootString").type() ) << std::endl;
		std::cout << '['<<doc.at("rootString")<<']' << std::endl;
		
		std::cout << P_TYPE( doc["main"].type() ) << std::endl;
		std::cout << '['<<doc["main"]<<']' << std::endl;

		Document	maindoc = doc["main"];
		std::cout << P_TYPE( maindoc["int"].type() ) << std::endl;
		std::cout << '['<<maindoc["int"]<<']' << std::endl;

		Document	subdoc = maindoc["sub"];
		std::cout << P_TYPE( subdoc["bool"].type() ) << std::endl;
		std::cout << '['<<subdoc["bool"]<<']' << std::endl;
	}
	catch (std::out_of_range const& e)
	{
		std::cout << REDB"Caught std::out_of_range: " << e.what() << RESET << std::endl;
	}
	newtest("Nested operator[]");
	try
	{
		Document	doc;
		doc.parse(argv[1]);
		std::cout << P_TYPE( doc.at("rootString").type() ) << std::endl;
		std::cout << '['<<doc.at("rootString")<<']' << std::endl;
		
		std::cout << P_TYPE( doc["main"]["sub"]["bool"].type() ) << std::endl;
		std::cout << '['<< doc["main"]["sub"]["bool"] <<']' << std::endl;
	}
	catch (std::out_of_range const& e)
	{
		std::cout << REDB"Caught std::out_of_range: " << e.what() << RESET << std::endl;
	}
	newtest("Nested at()");
	try
	{
		Document	doc;
		doc.parse(argv[1]);
		std::cout << P_TYPE( doc.at("rootString").type() ) << std::endl;
		std::cout << '['<<doc.at("rootString")<<']' << std::endl;
		
		std::cout << P_TYPE( doc.at("main").at("sub").at("bool").type() ) << std::endl;
		std::cout << '['<< doc.at("main").at("sub").at("bool") <<']' << std::endl;
	}
	catch (std::out_of_range const& e)
	{
		std::cout << REDB"Caught std::out_of_range: " << e.what() << RESET << std::endl;
	}
	newtest("Strings");
	{
		Document	doc(argv[1]);
		doc.parse();

		std::cout << doc["test"]["salut"] << std::endl;
		std::cout << doc["test"]["coucou"] << std::endl;
	}
	newtest("Array");
	{
		Document	doc(argv[1]);
		doc.parse();

		Document	test_a = doc["test"]["a"].Group();

		std::cout << P_TYPE( test_a["simple1"].type() ) << std::endl;
		std::cout << test_a["simple1"] << std::endl;

		std::cout << P_TYPE( test_a["simple2"].type() ) << std::endl;
		std::cout << test_a["simple2"] << std::endl;
		
		std::cout << P_TYPE( test_a["simple3"].type() ) << std::endl;
		std::cout << test_a["simple3"] << std::endl;

		std::cout << P_TYPE( test_a["simple4"].type() ) << std::endl;
		std::cout << test_a["simple4"] << std::endl;

		std::cout << P_TYPE( test_a["less-simple"].type() ) << std::endl;
		std::cout << test_a["less-simple"] << std::endl;

		std::cout << P_TYPE( test_a["wtf"].type() ) << std::endl;
		std::cout << test_a["wtf"] << std::endl;
	}
	newtest();
	{
		Document	doc(argv[1]);
		doc.parse();
		Document	test_a = doc["test"]["a"]; 

		std::cout << P_TYPE( test_a["simple1"].type() ) << std::endl;
		std::cout << test_a["simple1"] << std::endl;
		std::cout << test_a["simple1"][0] << std::endl;
		std::cout << test_a["simple1"][1] << std::endl;
		std::cout << test_a["simple1"][2] << std::endl;

		for (Value::array_iterator it = test_a["simple2"].begin(); it != test_a["simple2"].end(); ++it)
			std::cout << P_TYPE( it->type() ) << "\t" << *it << std::endl;

		for (Value::array_iterator it = test_a["simple3"].begin(); it != test_a["simple3"].end(); ++it)
			std::cout << P_TYPE( it->type() ) << "\t" << *it << std::endl;

		for (Value::array_iterator it = test_a["simple4"].begin(); it != test_a["simple4"].end(); ++it)
			std::cout << P_TYPE( it->type() ) << "\t" << *it << std::endl;
	}
	newtest("Array iterators");
	{
		Document	doc(argv[1]);
		doc.parse();
		Document	test_a = doc["test"]["a"];

		for (Value::array_iterator it = test_a["less-simple"].begin(); it != test_a["less-simple"].end(); ++it)
			std::cout << P_TYPE( it->type() ) << "\t" << *it << std::endl;
		
		for (Value::array_iterator it = test_a["wtf"].begin(); it != test_a["wtf"].end(); ++it)
			std::cout << P_TYPE( it->type() ) << "\t" << *it << std::endl;
	}
	newtest("Nested array operator[]");
	{
		Document	doc(argv[1]);
		doc.parse();
		Document	test_a = doc["test"]["a"];

		std::cout << test_a["less-simple"][0][0] << std::endl;
		std::cout << test_a["less-simple"][0][1] << std::endl;
		std::cout << test_a["less-simple"][1][0] << std::endl;

		std::cout << "===============================" << std::endl;

		std::cout << test_a["wtf"][0][0][0] << std::endl;
		std::cout << test_a["wtf"][0][1][0] << std::endl;
		std::cout << test_a["wtf"][1][0][0] << std::endl;
	}
	return 0;
}
