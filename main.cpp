#include "CSVParser.h"

int main(void)
{
    try
    {
        ifstream file("test.csv");
        CSVParser<int, string, string> parser(file, 0, '\n', ',', '\"');
        for (tuple<int, string, string> rs : parser)
        {
           cout << rs << endl;
        }
        file.close();
    }
    catch (const exception& ex)
    {
        cout << ex.what();
    }
    
    return 0;
}
