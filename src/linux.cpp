#include <argparse/argparse.hpp>

#include "common.h"

int main(int argc, const char* argv[]) {
	argparse::ArgumentParser cli{::PROJECT_NAME.data(), ::PROJECT_VERSION.data(), argparse::default_arguments::help};
	cli.add_argument("-i").help("Input file");
	cli.add_argument("-o").help("Output file");
	cli.add_argument("-s").help("Output size");
	cli.parse_args(argc, argv);

	if (!cli.is_used("-i") || !cli.is_used("-o")) {
		return 1;
	}
	const auto size = cli.is_used("-s") ? std::stoi(cli.get("-s")) : -1;
	return ::createThumbnail(cli.get("-i"), cli.get("-o"), size, size);
}
