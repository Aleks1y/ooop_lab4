#pragma once
#include "TupleUtils.h"

template<typename ... Args>
class CSVParser
{
private:
    ifstream& file;
    streamoff offsetPos;
    size_t offset;
    size_t linesCount = 0;
    char lineDelimiter;
    char columnDelimiter;
    char escapeCharacter;

    template <typename T>
    static T string_to_type(const string& src)
    {
        T value;
        stringstream s;
        s << src;
        s >> value;
        return value;
    };

    struct callback
    {
        template<typename T>
        void operator()(int index, T& t, vector<string>::iterator& it)
        {
            t = string_to_type<T>(*it);
            ++it;
        }
    };

    template<int index, typename Callback, typename... Args>
    struct iterate
    {
        static void next(tuple<Args...>& t, Callback callback, vector<string>::iterator& it)
        {
            iterate<index - 1, Callback, Args...>::next(t, callback, it);
            callback(index - 1, get<index>(t), it);
        }
    };

    template<typename Callback, typename... Args>
    struct iterate<0, Callback, Args...>
    {
        static void next(tuple<Args...>& t, Callback callback, vector<string>::iterator& it)
        {
            callback(0, get<0>(t), it);
        }
    };

    template<typename Callback, typename... Args>
    struct iterate<-1, Callback, Args...>
    {
        static void next(tuple<Args...>& t, Callback callback, vector<string>::iterator& it) {}
    };

    template<typename Callback, typename... Args>
    void forEach(tuple<Args...>& t, Callback callback, vector<string>::iterator& it)
    {
        const int size = tuple_size<tuple<Args...>>::value;
        iterate<size - 1, Callback, Args...>::next(t, callback, it);
    }

    tuple<Args...> vector_to_tuple(vector<string>& fields, size_t lineNumber)
    {
        tuple<Args...> tuple;
        if (tuple_size<std::tuple<Args...>>::value != fields.size())
            throw invalid_argument("Wrong number of fields in line " + to_string(lineNumber + 1) + "!");

        auto a = fields.begin();
        forEach(tuple, callback(), a);

        return tuple;
    }

    string readLine()
    {
        string str;
        char c;
        bool escapeReading = false;
        while (file.get(c) && (c != lineDelimiter || escapeReading))
        {
            if (c == escapeCharacter)
            {
                escapeReading = escapeReading ? 0 : 1;
            }
            str.push_back(c);
        }
        return str;
    }

    void readFields(const string& line, vector<string>& fields)
    {
        string str;
        bool escapeReading = false;
        for (char c : line)
        {
            if (c == escapeCharacter)
            {
                escapeReading = escapeReading ? 0 : 1;
            }
            else if (!escapeReading && c == columnDelimiter)
            {
                fields.push_back(str);
                str.clear();
            }
            else
            {
                str.push_back(c);
            }
        }

        fields.push_back(str);
    }

    class CSVIterator
    {
    private:
        CSVParser& parent;
        size_t index;
        vector<string> fields;

    public:
        CSVIterator(size_t index, CSVParser<Args...>& parent) : index(index), parent(parent)
        {
            if (index < parent.linesCount)
            {
                string line = parent.readLine();
                parent.readFields(line, fields);
            }
        }

        CSVIterator operator++()
        {
            index++;
            if (index < parent.linesCount)
            {
                fields.clear();
                string line = parent.readLine();
                parent.readFields(line, fields);
            }
            return *this;
        }

        bool operator==(const CSVIterator& other)
        {
            return this->index == other.index;
        }

        bool operator!=(const CSVIterator& other)
        {
            return !(*this == other);
        }

        tuple<Args...> operator*()
        {
            return parent.vector_to_tuple(fields, index);
        }
    };

public:
    CSVParser(ifstream& input, size_t offset, char lineDelimiter, char columnDelimiter, char escapeCharacter) : file(input), offset(offset), lineDelimiter(lineDelimiter), columnDelimiter(columnDelimiter), escapeCharacter(escapeCharacter)
    {
        if (!file.is_open())
            throw invalid_argument("Can't open file");
        if (offset < 0)
            throw logic_error("Bad file offset! offset < 0");

        char c;
        size_t k = 0;
        while (k < offset)
        {
            if (!file.get(c))
                throw logic_error("Bad file offset! offset >= file");

            if (c == lineDelimiter)
            {
                k++;
            }
        }

        linesCount += offset;
        offsetPos = file.tellg();
        string line;
        while (getline(file, line))
        {
            while (line.find(lineDelimiter) != string::npos)
            {
                linesCount++;
                line = line.substr(line.find(lineDelimiter) + 1);
            }

            if (lineDelimiter == '\n')
            {
                linesCount++;
            }
        }

        file.clear();
        file.seekg(offsetPos, file.beg);
    }

    CSVIterator begin()
    {
        file.seekg(offsetPos, file.beg);
        return CSVIterator(offset, *this);
    }

    CSVIterator end()
    {
        return CSVIterator(linesCount, *this);
    }
};
