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

#include <exception>

#include <seLib/experimental/CSVFile.h>

namespace seLib {
namespace CSVFile {

using namespace std;
using namespace seLib;

CSVReader::CSVReader() {
  readBuffer = new char[1024];
}

CSVReader::~CSVReader() {
  delete readBuffer;
  Close();
}

void CSVReader::Open(const string& fileName) {
  auto r = new ifstream(fileName);
  if (r->fail())
	  throw exception();
  reader = r;
  FileName = fileName;
}

void CSVReader::Open(istream& source) {
  reader = &source;
}

void CSVReader::Close() {
  if (reader != nullptr && !FileName.empty())
    delete reader;
  reader = nullptr;
}

bool CSVReader::ReadLine(unsigned int maxCellCount) {
  row.clear();
  if (reader->eof())
    return false;

  int count = 0;
  reader->getline(readBuffer, 1024);
  char* start = readBuffer;
  char* end = readBuffer + reader->gcount() - 1;
  if (start > end)
    return true;

  *(end + 1) = 0;

  bool tabIsWhiteSpace = (Separator != '\t');
  bool skipnext = false;
  bool inquote = false;
  char* lastvalid = nullptr;

  for (; start <= end; start++) {
    char c = *start;
    if (skipnext && c != 0) {
      lastvalid = start;
      continue;
    }
    if (c == '\\') {
      lastvalid = start;
      skipnext = true;
      continue;
    }
    if (c == Separator || (inquote && c == '\"'))
      *start = 0;
    if (*start == 0) {
      if (lastvalid != nullptr)
        *(lastvalid + 1) = 0;
      lastvalid = nullptr;
      inquote = false;
      skipnext = false;
      continue;
    }
    if (!inquote && c == '\"') {
      inquote = true;
    } else if (c != ' ' && (!tabIsWhiteSpace || c != '\t')) {
      if (lastvalid == nullptr) {
        row.push_back(start);
        if (maxCellCount > 0 && row.size() > maxCellCount)
          return true;
      }
      lastvalid = start;
    }

  }

  return true;
}

//============================================================================


CSVFileWriter::CSVFileWriter(string fileName) {
  writer = new ofstream(fileName);
  FileName = fileName;
}

CSVFileWriter::~CSVFileWriter() {
  Close();
}

void CSVFileWriter::Flush() {
  writer->flush();
}

void CSVFileWriter::Close() {
  Flush();
  if (writer != nullptr && !FileName.empty())
    delete writer;
  writer = nullptr;
}

void CSVFileWriter::WriteLine(vector<string> data) {
  string line = "";
  for (int i = 0; i < (int)data.size(); i++) {
    if (i > 0)
      *writer << Separator;

    string& s = data[i];
    //double d;
    try {
      stod(s); // see if string is parseable number, enclose in quotes otherwise
      writer->write(s.c_str(), s.size());
    } catch (const std::invalid_argument& ia) {
      *writer << '\"';
      writer->write(s.c_str(), s.size());
      *writer << '\"';
    }
  }
}

}
}
