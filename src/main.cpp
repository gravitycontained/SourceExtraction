#include <qpl/qpl.hpp>

constexpr auto invalid_directories = std::array{
	".vs", "Debug", "Release", "x64", "x86"
};
namespace data {
	std::string source_name = "SRC_EXTRACT";
}

qpl::size get_input_size() {
	qpl::size result = qpl::size_max;
	while (true) {
		qpl::print("enter maximum file size [enter to ignore] > ");

		auto input = qpl::get_input();
		if (input.empty()) {
			result = qpl::size_max;
			break;
		}
		auto split = qpl::split_string_digit_alpha(input);
		if (split.size() == 2u) {

			constexpr auto by = 1.0;
			constexpr auto bi = 3.0;

			auto amount = qpl::f64_cast(split[0u]);
			auto type = split[1u];

			if (qpl::string_equals_ignore_case(type, "b")) {
				result = qpl::size_cast(amount);
			}
			else if (qpl::string_equals_ignore_case(type, "kb")) {
				result = qpl::size_cast(amount * qpl::pow(10.0, bi * 1.0));
			}
			else if (qpl::string_equals_ignore_case(type, "kib")) {
				result = qpl::size_cast(amount * qpl::pow(1024.0, by * 1.0));
			}
			else if (qpl::string_equals_ignore_case(type, "mb")) {
				result = qpl::size_cast(amount * qpl::pow(10.0, bi * 2.0));
			}
			else if (qpl::string_equals_ignore_case(type, "mib")) {
				result = qpl::size_cast(amount * qpl::pow(1024.0, by * 2.0));
			}
			else if (qpl::string_equals_ignore_case(type, "gb")) {
				result = qpl::size_cast(amount * qpl::pow(10.0, bi * 3.0));
			}
			else if (qpl::string_equals_ignore_case(type, "gib")) {
				result = qpl::size_cast(amount * qpl::pow(1024.0, by * 3.0));
			}
			else if (qpl::string_equals_ignore_case(type, "tb")) {
				result = qpl::size_cast(amount * qpl::pow(10.0, bi * 4.0));
			}
			else if (qpl::string_equals_ignore_case(type, "tib")) {
				result = qpl::size_cast(amount * qpl::pow(1024.0, by * 4.0));
			}
			else {
				return get_input_size();
			}

			break;
		}
		else if (split.size() == 1u) {
			result = qpl::size_cast(split[0u]);
			break;
		}
	}
	return result;
}

void extract_directory_recursive(const qpl::filesys::paths& paths, qpl::size max_size, qpl::filesys::path& current_path, qpl::filesys::path& destination_path) {
	for (auto& path : paths) {

		bool valid = true;
		if (path.is_directory()) {
			auto name = path.get_full_name();
			for (auto& invalid : invalid_directories) {
				if (name == invalid) {
					valid = false;
					break;
				}
			}
		}

		if (valid) {
			if (path.is_directory()) {
				current_path.go_into(path.get_full_name());
				destination_path.go_into(path.get_full_name());
				destination_path.ensure_branches_exist();
				auto paths = current_path.list_current_directory();
				extract_directory_recursive(paths, max_size, current_path, destination_path);
				destination_path.go_directory_back();
				current_path.go_directory_back();
			}
			else if (path.is_file()) {
				if (path.file_size() <= max_size) {
					qpl::filesys::copy_overwrite(path, destination_path.string() + path.get_full_name());
				}
				//qpl::println("COPY ", path, " -> ", destination_path);
			}
		}
	}
}

void extract_directory(std::string name, std::string path_string, qpl::size max_size) {
	qpl::filesys::path current_path = path_string;

	if (current_path.is_file()) {
		qpl::println("rejected ", path_string, ": not a directory.");
		return;
	}
	if (current_path.is_directory() && current_path.get_directory_name().starts_with(data::source_name)) {
		qpl::println("rejected ", path_string, ": is a source extract.");
		return;
	}

	bool valid = false;
	auto check = current_path.list_current_directory_tree();
	for (auto& i : check) {
		if (i.is_file() && i.get_file_extension() == "sln") {
			valid = true;
			break;
		}
	}
	if (!valid) {
		qpl::println("rejected ", path_string, ": no solution file found.");
		return;
	}
	qpl::print("extracting ", path_string, " . . . ");

	qpl::filesys::path destination_path = name;
	destination_path.go_into(current_path.get_full_name());

	auto paths = current_path.list_current_directory();
	extract_directory_recursive(paths, max_size, current_path, destination_path);

	qpl::println("done");
}

int main(int argc, char** argv) try {
	std::vector<std::string> args(argc - 1);
	for (qpl::size i = 0u; i < argc - 1; ++i) {
		args[i] = argv[i + 1];
	}

	qpl::size ctr = 0u;
	qpl::filesys::path name = data::source_name;

	if (name.exists()) {
		while (true) {
			qpl::print(name, " already exists. overwrite or set new name? [enter to set new name] (o/n) > ");
			auto input = qpl::get_input();
			if (qpl::string_equals_ignore_case(input, "o")) {
				break;
			}
			if (input.empty() || qpl::string_equals_ignore_case(input, "n")) {
				while (name.exists()) {
					name = qpl::to_string(data::source_name, ctr);
					++ctr;
				}
				break;
			}
		}
	}


	qpl::println();
	qpl::println_repeat("- ", 50);
	qpl::println();
	
	qpl::size max_size = get_input_size();

	qpl::clock clock;
	if (argc == 1) {
		auto paths = qpl::filesys::list_current_directory();
		for (auto path : paths) {
			extract_directory(name, path, max_size);
		}
	}
	else {
		for (auto path : args) {
			extract_directory(name, path, max_size);
		}
	}
	auto elapsed = clock.elapsed_str();
	auto tree = qpl::filesys::get_current_location();

	qpl::println();
	qpl::println_repeat("- ", 50);
	qpl::println();
	tree.go_into(name);
	qpl::println(tree, " has ", qpl::memory_size_string(tree.file_size_recursive()), " size.");
	qpl::println();
	tree.list_current_directory().print_tree();

	qpl::println();
	qpl::println_repeat("- ", 50);
	qpl::println();
	qpl::println("took ", elapsed);
	qpl::system_pause();


}
catch (std::exception& any) {
	qpl::println("exception caught: ");
	qpl::println(any.what());
	qpl::system_pause();
}