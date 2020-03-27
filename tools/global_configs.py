import os, infra_lib

configs = infra_lib.Object();
configs.compiler_options = infra_lib.Object();
configs.package = "aparse-1.7"
configs.prod_cc_flags = " ".join([" -Wno-unused-function ",
                                  " -Wno-unused-parameter ",
                                  " -Wno-unused-local-typedefs ",
                                  " -Wno-unused-variable "]);
configs.global_include_dir = ["include", "."];
configs.active_remote_branch = "dev-aparse-1.7";
toolchain_path = configs.toolchain_path = os.path.join(os.environ["HOME"], "toolchain");
projects_path = os.environ["HOME"] + "/projects"

br = infra_lib.BuildRule();
configs.dependency_configs = [

  br.CppLibrary(
      "toolchain/gtest",
      srcs = [ toolchain_path + "/googletest-release-1.8.1/googletest/src/gtest-all.cc" ],
      global_include_dir = [ toolchain_path + "/googletest-release-1.8.1/googletest/include" ],
      local_include_dir = [ toolchain_path+ "/googletest-release-1.8.1/googletest" ],
      global_link_flags = "-lpthread"),

  br.CppLibrary("toolchain/json11",
                      srcs = [toolchain_path + "/json11-original/json/json11.cpp"],
                      global_include_dir = [ toolchain_path + "/json11-original"]),

  br.CppLibrary("toolchain/quick",
                      srcs = [projects_path + "/quick/src/quick-all.cpp"],
                      global_include_dir = [projects_path + "/quick/include"]),

  # br.CppLibrary("src/lexer",
  #               hdrs = ["include/aparse/lexer.hpp"],
  #               srcs = ["src/lexer.cpp"]),


  # br.CppLibrary("src/aparse",
  #               hdrs = ["include/aparse/aparse.hpp"],
  #               srcs = ["src/aparse.cpp"],
  #               deps = ["toolchain/quick"]),

  br.CppLibrary("tests/samples/sample_internal_parser_rules",
                hdrs = ["tests/samples/sample_internal_parser_rules.hpp"],
                deps = ["src/simple_aparse_grammar_builder"]),

  br.CppLibrary("src/parse_char_regex_rules",
                hdrs = ["src/parse_char_regex_rules.hpp"],
                srcs = ["src/parse_char_regex_rules.cpp"],
                deps = ["src/regex_builder"]),

  br.CppLibrary("src/utils",
                hdrs = ["src/utils.hpp"],
                srcs = ["src/utils.cpp"],
                deps = ["toolchain/quick"]),

  br.CppLibrary("aparse/utils/any",
                hdrs = ["include/aparse/utils/any.hpp"],
                deps = ["toolchain/quick"]),

  br.CppLibrary("src/parse_char_regex",
                hdrs = ["src/parse_char_regex.hpp"],
                srcs = ["src/parse_char_regex.cpp"],
                deps = ["src/parse_char_regex_rules",
                        "src/internal_parser_builder"]),

  br.CppLibrary("src/parse_regex_rule",
                hdrs = ["src/parse_regex_rule.hpp"],
                srcs = ["src/parse_regex_rule.cpp"],
                deps = ["src/internal_lexer_builder",
                        "aparse/parser",
                        "src/parse_char_regex"]),

  br.CppTest("src/parse_char_regex_test",
                srcs = ["src/parse_char_regex_test.cpp"],
                deps = ["src/parse_char_regex"]),

  br.CppTest("src/parse_regex_rule_test",
                srcs = ["src/parse_regex_rule_test.cpp"],
                deps = ["src/parse_regex_rule"]),

  br.CppLibrary("aparse/regex",
                hdrs = ["include/aparse/regex.hpp"],
                srcs = ["src/regex.cpp"],
                deps = ["toolchain/quick",
                        "src/helpers"]),

  br.CppLibrary("src/regex_helpers",
                hdrs = ["src/regex_helpers.hpp"],
                srcs = ["src/regex_helpers.cpp"],
                deps = ["toolchain/quick",
                        "aparse/regex"]),

  br.CppLibrary("src/helpers",
                hdrs = ["src/helpers.hpp"],
                srcs = ["src/helpers.cpp"],
                deps = ["src/utils"]),

  br.CppLibrary("aparse/error",
                hdrs = ["include/aparse/error.hpp"],
                srcs = ["src/error.cpp"]),

  br.CppLibrary("aparse/aparse_grammar",
                hdrs = ["include/aparse/aparse_grammar.hpp"],
                srcs = ["src/aparse_grammar.cpp"],
                deps = ["toolchain/quick",
                        "aparse/error",
                        "aparse/regex",
                        "src/regex_helpers",
                        "src/helpers"]),

  br.CppLibrary("src/v2/aparse_machine",
                hdrs = ["src/v2/aparse_machine.hpp"],
                srcs = ["src/v2/aparse_machine.cpp"],
                deps = ["toolchain/quick"]),

  # br.CppLibrary("src/v1/aparse_machine_builder",
  #               hdrs = ["src/v1/aparse_machine_builder.hpp"],
  #               srcs = ["src/v1/aparse_machine_builder.cpp"],
  #               deps = ["aparse/aparse_grammar",
  #                       "src/v1/aparse_machine"]),

  br.CppLibrary("src/v2/internal_aparse_grammar",
                hdrs = ["src/v2/internal_aparse_grammar.hpp"],
                srcs = ["src/v2/internal_aparse_grammar.cpp"],
                deps = ["aparse/aparse_grammar",
                        "src/utils"]),

  br.CppLibrary("src/v2/aparse_machine_builder",
                hdrs = ["src/v2/aparse_machine_builder.hpp"],
                srcs = ["src/v2/aparse_machine_builder.cpp"],
                deps = ["aparse/aparse_grammar",
                        "src/v2/aparse_machine",
                        "src/v2/internal_aparse_grammar"]),

  # br.CppTest("src/v1/aparse_machine_test",
  #               srcs = ["src/v1/aparse_machine_test.cpp"],
  #               deps = ["src/v1/aparse_machine"]),

  br.CppLibrary("aparse/core_parse_node",
                hdrs = ["include/aparse/core_parse_node.hpp"],
                srcs = ["src/core_parse_node.cpp"],
                deps = ["toolchain/quick",
                        "aparse/error"]),

  br.CppLibrary("src/abstract_core_parser",
                hdrs = ["src/abstract_core_parser.hpp"],
                deps = ["toolchain/quick",
                        "aparse/error",
                        "aparse/core_parse_node"]),

  br.CppLibrary("src/v2/core_parser",
                hdrs = ["src/v2/core_parser.hpp"],
                srcs = ["src/v2/core_parser.cpp"],
                deps = ["src/v2/aparse_machine",
                        "aparse/error",
                        "src/abstract_core_parser"]),

  br.CppLibrary("aparse/lexer_machine",
                hdrs = ["include/aparse/lexer_machine.hpp"],
                deps = []),

  br.CppLibrary("src/lexer_machine_builder",
                hdrs = ["src/lexer_machine_builder.hpp"],
                srcs = ["src/lexer_machine_builder.cpp"],
                deps = ["aparse/lexer_machine",
                        "aparse/regex"]),

  br.CppLibrary("aparse/parser_builder",
                hdrs = ["include/aparse/parser_builder.hpp"],
                srcs = ["src/parser_builder.cpp"],
                deps = ["aparse/parser",
                        "src/internal_parser_builder",
                        "src/parse_regex_rule",
                        "aparse/error"]),

  br.CppLibrary("aparse/lexer",
                hdrs = ["include/aparse/lexer.hpp"],
                srcs = ["src/lexer.cpp"],
                deps = ["aparse/error"]),

  br.CppLibrary("aparse/lexer_builder",
                hdrs = ["include/aparse/lexer_builder.hpp"],
                srcs = ["src/lexer_builder.cpp"],
                deps = ["aparse/lexer",
                        "src/internal_lexer_builder",
                        "src/parse_char_regex"]),

  br.CppLibrary("src/internal_lexer_builder",
                hdrs = ["src/internal_lexer_builder.hpp"],
                srcs = ["src/internal_lexer_builder.cpp"],
                deps = ["src/lexer_machine_builder",
                        "aparse/error",
                        "aparse/lexer",
                        "aparse/regex"]),

  br.CppLibrary("src/internal_parser_builder",
                hdrs = ["src/internal_parser_builder.hpp"],
                srcs = ["src/internal_parser_builder.cpp"],
                deps = ["aparse/parser",
                        "src/v2/core_parser",
                        "toolchain/quick",
                        "src/v2/aparse_machine_builder"]),

  br.CppLibrary("aparse/parser",
                hdrs = ["include/aparse/parser.hpp"],
                srcs = ["src/parser.cpp"],
                deps = ["src/v2/aparse_machine",
                        "aparse/error",
                        "src/v2/core_parser",
                        "toolchain/quick"]),

  br.CppLibrary("src/regex_builder",
                hdrs = ["src/regex_builder.hpp"],
                srcs = ["src/regex_builder.cpp"],
                deps = ["toolchain/quick",
                        "aparse/regex"]),

  br.CppLibrary("src/simple_aparse_grammar_builder",
                hdrs = ["src/simple_aparse_grammar_builder.hpp"],
                srcs = ["src/simple_aparse_grammar_builder.cpp"],
                deps = ["toolchain/quick",
                        "src/regex_builder",
                        "aparse/aparse_grammar"]),

  br.CppLibrary("aparse/aparse",
                hdrs = ["include/aparse/aparse.hpp"],
                srcs = ["src/aparse.cpp"],
                deps = ["toolchain/quick"]),

  br.CppTest("src/regex_test",
                srcs = ["src/regex_test.cpp"],
                deps = ["aparse/regex"]),

  br.CppTest("src/v2/core_parser_integration_test",
                srcs = ["src/v2/core_parser_integration_test.cpp"],
                deps = ["src/v2/core_parser",
                        "src/v2/aparse_machine_builder"]),

  # br.CppTest("tests/bug1_test",
  #               srcs = ["tests/bug1_test.cpp"],
  #               deps = ["src/core_parser",
  #                       "src/aparse_machine_builder_v2",
  #                       "src/parse_regex_rule"]),


  # br.CppTest("src/internal_parser_integration_test",
  #               srcs = ["src/internal_parser_integration_test.cpp"],
  #               deps = ["aparse/parser"]),

  br.CppTest("src/internal_lexer_builder_integration_test",
                srcs = ["src/internal_lexer_builder_integration_test.cpp"],
                deps = ["src/internal_lexer_builder",
                        "toolchain/quick",
                        "aparse/regex",
                        "src/regex_helpers"]),

  br.CppTest("src/lexer_builder_integration_test",
                srcs = ["src/lexer_builder_integration_test.cpp"],
                deps = ["aparse/lexer_builder",
                        "toolchain/quick"]),

  br.CppTest("src/parser_builder_integration_test",
                srcs = ["src/parser_builder_integration_test.cpp"],
                deps = ["aparse/parser_builder",
                        "aparse/lexer_builder",
                        "toolchain/quick"]),

  br.CppTest("src/v2/internal_aparse_grammar_test",
                srcs = ["src/v2/internal_aparse_grammar_test.cpp"],
                deps = ["src/v2/internal_aparse_grammar",
                        "src/utils"]),

  br.CppTest("src/v2/aparse_machine_test",
                srcs = ["src/v2/aparse_machine_test.cpp"],
                deps = ["src/v2/aparse_machine"]),

  br.CppTest("src/regex_builder_test",
                srcs = ["src/regex_builder_test.cpp"],
                deps = ["src/regex_builder"]),

  br.CppTest("src/simple_aparse_grammar_builder_test",
                srcs = ["src/simple_aparse_grammar_builder_test.cpp"],
                deps = ["src/simple_aparse_grammar_builder"]),

  br.CppTest("src/v2/aparse_machine_builder_test",
                srcs = ["src/v2/aparse_machine_builder_test.cpp"],
                deps = ["src/v2/aparse_machine_builder",
                        "src/regex_builder"]),

  br.CppTest("src/internal_parser_builder_integration_test",
                srcs = ["src/internal_parser_builder_integration_test.cpp"],
                deps = ["src/internal_parser_builder",
                        "tests/samples/sample_internal_parser_rules"]),

  br.CppTest("tests/integration_tests/aparse_test",
                srcs = ["tests/integration_tests/aparse_test.cpp"],
                deps = ["aparse/aparse",
                        "toolchain/quick"]),

  br.CppTest("src/utils/any_test",
                srcs = ["src/utils/any_test.cpp"],
                deps = ["aparse/utils/any"]),

  br.CppProgram("tools/experiments/parser1",
                ignore_cpplint = True,
                srcs = ["tools/experiments/parser1.cpp"],
                deps = ["src/internal_parser",
                        "toolchain/quick"]),

  br.CppLibrary("tools/experiments/try2",
                ignore_cpplint = True,
                srcs = ["tools/experiments/try2.cpp"],
                deps = ["toolchain/quick"]),

  br.CppProgram("tools/experiments/try",
                ignore_cpplint = True,
                srcs = ["tools/experiments/try.cpp"],
                deps = ["toolchain/quick",
                        "tools/experiments/try2"]),

  # br.CppTest("tools/experiments/try_test",
  #               ignore_cpplint = True,
  #               srcs = ["tools/experiments/try_test.cpp"],
  #               deps = ["toolchain/quick"]),

  br.CppProgram("tools/experiments/dev",
                srcs = ["tools/experiments/dev.cpp"],
                deps = ["toolchain/quick",
                        "src/v2/aparse_machine_builder",
                        "src/regex_builder",
                        "src/parse_regex_rule"],
                ignore_cpplint = True,
                local_include_dir = ["."]),
];

configs["LINKFLAGS"] = ""
if (os.environ.get("DEV_MACHINE") == "ts"):
  configs["CXX"] = "/usr/local/scaligent/toolchain/crosstool/v4/x86_64-unknown-linux-gnu/bin/x86_64-unknown-linux-gnu-g++";
  configs["LINKFLAGS"] = "-Wl,--compress-debug-sections=zlib -Wl,--dynamic-linker=/usr/local/scaligent/toolchain/crosstool/v4/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/sysroot/lib/ld-linux-x86-64.so.2 -B/usr/local/scaligent/toolchain/crosstool/v4/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/bin.gold -Wl,-rpath=/usr/local/scaligent/toolchain/crosstool/v4/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/sysroot/lib -Wl,--no-whole-archive ";

def SetCompilerOps(compiler_options):
  configs.compiler_options.update(compiler_options);
  configs["CCFLAGS"] = "--std=c++14 -Wall -Wextra -Wno-sign-compare -fno-omit-frame-pointer -Wnon-virtual-dtor -mpopcnt -msse4.2 -g3 -Woverloaded-virtual -Wno-char-subscripts -Werror=deprecated-declarations -Wa,--compress-debug-sections -fdiagnostics-color=always  -Werror ";
  #  -Wpedantic  -Wextra 
  if (configs.compiler_options["mode"] == "debug"):
    configs["CCFLAGS"] += " -O0 ";
  elif (configs.compiler_options["mode"] == "opt"):
    configs["CCFLAGS"] += " -O3 " + configs.prod_cc_flags;
  elif (configs.compiler_options["mode"] == "release"):
    configs["CCFLAGS"] += " -O3 " + configs.prod_cc_flags;

SetCompilerOps(dict(mode = "opt"));

configs.SetCompilerOps = SetCompilerOps;


