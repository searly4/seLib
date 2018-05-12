#pragma once
/*
   Copyright 2018 by Scott Early

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <vector>
#include <string_view>

namespace seLib {
namespace ParseArgs {

using namespace std;

struct ArgDefBase {
	string_view ArgText;
};

template <typename ArgData_T>
struct ArgDef : public ArgDefBase {
	string_view ArgText;
	ArgData_T& Data;
};

template <typename T>
struct ArgDef_ValueType : public ArgDef<ArgDef_ValueType<T>> {
	int OptionCount = 0;
};

struct ArgData {
	bool ArgPresent = false;
};

template <typename T>
struct ArgData_ValueType {
	vector<vector<T>> Strings;
};

class ParseArgs {
public:
	vector<const ArgDef*> ArgDefs;
	vector<string_view> UnattachedStrings;

	ParseArgs(const vector<const ArgDef*>& arg_defs) :
		ArgDefs(arg_defs)
	{ }

	void Parse(int argc, const char* argv[]) {
		for (int i = 1; i < argc; i++) {
			const char* arg = argv[i];

			if (arg[0] == '-') {
				string comp(toLower(arg + 1));

				if (comp == "h") {
					printf("FilterTest [-h] [-adc <path>] [-bpf <lower> <upper>] [-lpf <freq>] [-hpf <freq>] [<outfile>]");

				} else if (comp == "adc") {
					arg_read_adc_set = true;
					i++;
					adc_set_path = argv[i];
					out_file_name = string(argv[i]) + "\\" + out_file_name;

				} else if (comp == "bpf") {
					i++;
					float lf = std::stof(argv[i]);
					i++;
					float uf = std::stof(argv[i]);
					filters.emplace_back(FIR_Filter::BandPass(taps, freq, lf, uf));

				} else if (comp == "lpf") {
					i++;
					float f = std::stof(argv[i]);
					filters.emplace_back(FIR_Filter::LowPass(taps, freq, f));

				} else if (comp == "hpf") {
					i++;
					float f = std::stof(argv[i]);
					filters.emplace_back(FIR_Filter::HighPass(taps, freq, f));
				}
			}
		}
	}
};


}
}
