#include <iostream>
#include <string>
using namespace std;
int main() {
    std::string myString = "Hello, World!";
    
    // Change the entire content of the string
    myString = "";
    
    cout << myString<<"str length: " << myString.length()<<endl;  // Output: New entire content
    
    return 0;
}
