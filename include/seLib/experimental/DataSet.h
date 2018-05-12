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

#include <string_view>
#include <string>

namespace seLib {

using namespace std;

class DataSet {
public:
	virtual size_t size() = 0;
	virtual string Name() = 0;
	virtual string Name(size_t index) = 0;
	virtual string to_string(size_t index) = 0;
};

template <typename T>
class ScalarDataSet : public DataSet {
protected:
	string _Name;
public:
	string ValueName;
	T Value;

	ScalarDataSet() : _Name("") {}

	ScalarDataSet(string name) : _Name(name) {}

	size_t size() override {
		return 1;
	}

	string Name() override {
		return _Name;
	}

	string Name(size_t index) override {
		return "Value";
	}

	string to_string(size_t index) override {
		return std::to_string(Value);
	}
};

template <typename T>
class ArrayDataSet : public DataSet {
protected:
	string _Name;
public:
	vector<T> Data;

	ArrayDataSet() : _Name("") {}

	ArrayDataSet(string name) : _Name(name) {}

	size_t size() override {
		return Data.size();
	}

	string Name() override {
		return _Name;
	}

	string Name(size_t index) override {
		return std::to_string(index);
	}

	string to_string(size_t index) override {
		return std::to_string(Data[index]);
	}
};

}
