#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "users.txt"

// Using struct keyword to represent user
typedef struct {
    int userId;
    char userName[50];
    int userAge;
} User;

// To add a new user
void addUser() {
    FILE *filePtr = fopen(DATA_FILE, "a");
    if (!filePtr) {
        printf("Error: not able to open the file.\n");
        return;
    }

    User newUser;
    printf(" Add a new user ");
    printf("Enter user ID: ");
    scanf("%d", &newUser.userId);
    printf("Enter name: ");
    scanf("%s", newUser.userName);
    printf("Enter age: ");
    scanf("%d", &newUser.userAge);

    fprintf(filePtr, "%d %s %d\n", newUser.userId, newUser.userName, newUser.userAge);
    fclose(filePtr);

    printf("The user has been added successfullly \n");
}

// To display existing users
void showUsers() {
    FILE *filePtr = fopen(DATA_FILE, "r");
    if (!filePtr) {
        printf("File is empty.\n");
        return;
    }

    User currentUser;
    printf(" User List ");
    while (fscanf(filePtr, "%d %s %d", &currentUser.userId, currentUser.userName, &currentUser.userAge) == 3) {
        printf("ID: %-3d | Name: %-15s | Age: %d\n", currentUser.userId, currentUser.userName, currentUser.userAge);
    }

    fclose(filePtr);
}

// To update an existing user
void updateUser() {
    FILE *filePtr = fopen(DATA_FILE, "r");
    if (!filePtr) {
        printf("Invalid user ID.\n");
        return;
    }

    FILE *tempFile = fopen("temp.txt", "w");
    if (!tempFile) {
        fclose(filePtr);
        printf("Error: could not create temporary file.\n");
        return;
    }

    int searchId, found = 0;
    printf("\nEnter the ID of the user to update: ");
    scanf("%d", &searchId);

    User currentUser;
    while (fscanf(filePtr, "%d %s %d", &currentUser.userId, currentUser.userName, &currentUser.userAge) == 3) {
        if (currentUser.userId == searchId) {
            found = 1;
            printf("Updating user with ID %d\n", currentUser.userId);
            printf("Enter the new name: ");
            scanf("%s", currentUser.userName);
            printf("Enter the new age: ");
            scanf("%d", &currentUser.userAge);
        }
        fprintf(tempFile, "%d %s %d\n", currentUser.userId, currentUser.userName, currentUser.userAge);
    }

    fclose(filePtr);
    fclose(tempFile);
    remove(DATA_FILE);
    rename("temp.txt", DATA_FILE);

    if (found) {
        printf("User has been updated succesfully.\n");
    } else {
        printf("There is no such user with ID %d.\n", searchId);
    }
}

// To delete an existing user
void deleteUser() {
    FILE *filePtr = fopen(DATA_FILE, "r");
    if (!filePtr) {
        printf("No data file found.\n");
        return;
    }

    FILE *tempFile = fopen("temp.txt", "w");
    if (!tempFile) {
        fclose(filePtr);
        printf("Error: could not create temporary file.\n");
        return;
    }

    int searchId, found = 0;
    printf("\nEnter the ID of the user to delete from the file: ");
    scanf("%d", &searchId);

    User currentUser;
    while (fscanf(filePtr, "%d %s %d", &currentUser.userId, currentUser.userName, &currentUser.userAge) == 3) {
        if (currentUser.userId == searchId) {
            found = 1;
            continue; // To skip this record
        }
        fprintf(tempFile, "%d %s %d\n", currentUser.userId, currentUser.userName, currentUser.userAge);
    }

    fclose(filePtr);
    fclose(tempFile);
    remove(DATA_FILE);
    rename("temp.txt", DATA_FILE);

    if (found) {
        printf("User has been deleted successfully\n");
    } else {
        printf("No such user exist with ID %d.\n", searchId);
    }
}

// Function to print menu options
void printMenu() {
    printf("User Management System\n");
    printf("\n");
    printf("1. Add User\n");
    printf("2. Show All Users\n");
    printf("3. Update User\n");
    printf("4. Delete User\n");
    printf("5. Exit\n");
}

// Function to handle menu choice
void handleMenu(int choice) {
    switch (choice) {
        case 1: addUser(); break;
        case 2: showUsers(); break;
        case 3: updateUser(); break;
        case 4: deleteUser(); break;
        case 5:
            printf("To exit the program\n");
            exit(0);
        default:
            printf("The choice is Invalid. Please try again.\n");
    }
}

int main() {
    int userChoice;

    while (1) {
        printMenu();
        printf("Choose an option: ");
        scanf("%d", &userChoice);
        handleMenu(userChoice);
    }

    return 0;
}
