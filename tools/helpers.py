#!/usr/bin/env python3

import tools.infra_lib as infra_lib
import os

inf = infra_lib;  # Don't use this shortcut in permanent code. It can be used for experimenting.

def PreProcessDependencyConfigs(configs):
  for i in configs.dependency_configs:
    if i["type"] == "CppTest":
      i["deps"].append("tests/test_main");
  br = infra_lib.BuildRule();
  configs.dependency_configs.append(
    br.CppLibrary("tests/test_main",
                  srcs = ["tests/test_main.cpp"],
                  deps = ["toolchain/gtest"]));
  return configs;


def RunLintChecks(configs, files = None):
  if (files == None):
    files = infra_lib.CppSourceFilesList(
      configs,
      filter = (lambda x: x.get("ignore_cpplint") != True));
  infra_lib.RunLinuxCommand(os.path.join(configs.toolchain_path + "/cpplint.py") + " --filter=-build/header_guard,-readability/alt_tokens " + " ".join(files));

def RunAllTests(configs, pp = 8, tests = None):
  if (tests == None):
    tests = list(i["name"] for i in configs.dependency_configs if i["type"] == "CppTest")
  infra_lib.RunLinuxCommand("scons -k -j"+str(pp) + " mode=" + configs.compiler_options.mode + " " + " ".join(tests));
  for i in tests:
    infra_lib.RunLinuxCommand("./" + configs.build_dir + "/" + i);

def ReplaceAllInFile(str1, str2, file):
  inf.WriteFile(file, inf.ReadFile(file).replace(str1, str2));


def Migration1():
  to_replace = {"Epsilon": "EPSILON",
                "Atomic": "ATOMIC",
                "Union": "UNION",
                "Concat": "CONCAT",
                "KleeneStar": "KSTAR",
                "KleenePlus": "KPLUS"};
  for i in (inf.AllFilesRecursive("src") + inf.AllFilesRecursive("include")):
    for k,v in to_replace.items():
      ReplaceAllInFile("Regex::"+k, "Regex::"+v, i);
    print("Done for " + str(i));
