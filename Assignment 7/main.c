#include "Players_data.h"

//We are calling the functions over here so that the compiler knows these functions exists somewhere
void initializeData();
void displayTheMenu();
int main() {
    //Calling of the required functions
    //Keeping the main clean and minimal
    initializeData();
    displayTheMenu();
}
