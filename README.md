# toml_parser
This is just my small attempt at a very basic TOML parser, to be used in the 42 School "webserv" project. As such, it doesn't nor will support a lot of TOML features, being TOML 0.2.0 compliant, except for dates. There was no need for more, after all.

It's my first attempt at making an object oriented parser, I mainly inspired myself from RapidJSON, which I used in another project.

This parser aims to be:
- Header-only.
- Easy to get used to, and to navigate your TOML document with.

It is, however, quite limited by C++98, a restriction placed upon us by that one rule in webserv's subject. Not really great.

The parser resides in two classes:
- The Document class:
	- Takes in the path to a TOML file, and parses it.
	- Can access individual `TOML::Values` with an unprotected `operator[]`, or a protected `at()` method.
	- You can chain these calls to access nested Values.
- The Value class:
	- Represents a key/value pair.
	- The type of the value can be an integer, a floating-point, a boolean, a string, or an array (_TODO:_ Dates).
	- A `Value` can also represent a key group, which can contain other key/value pairs.
	- The `Value` class implements an `operator<<` overload, for easier debugging and output. It only outputs the value, not the key. You've got a `key()` method for that.

For more information, you can look at the header files, which are actually readable, since implementation is separated into .ipp files.

Keep in mind that this library was made specifically to be integrated in another project (webserv). So it's not the most generalistic thing ever, but it still should be easy to be used elsewhere.
