#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "users.txt"

// Using struct keyword to represent user
typedef struct {
    int id;
    char name[50];
    int age;
} User;

// To add a new user
void create() {
    FILE *fp = fopen(DATA_FILE, "a");
    if (!fp) {
        printf("Error: not able to open the file.\n");
        return;
    }

    User u;
    printf(" Add a new user ");
    printf("Enter user ID: ");
    scanf("%d", &u.id);
    printf("Enter name: ");
    scanf("%s", u.name);
    printf("Enter age: ");
    scanf("%d", &u.age);

    fprintf(fp, "%d %s %d\n", u.id, u.name, u.age);
    fclose(fp);

    printf("The user has been added successfullly \n");
}

// To display existing users
void display() {
    FILE *fp = fopen(DATA_FILE, "r");
    if (!fp) {
        printf("File is empty.\n");
        return;
    }

    User u;
    printf(" User List ");
    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        printf("ID: %-3d | Name: %-15s | Age: %d\n", u.id, u.name, u.age);
    }

    fclose(fp);
}

// To update an existing user
void update() {
    FILE *fp = fopen(DATA_FILE, "r");
    if (!fp) {
        printf("Invalid user ID.\n");
        return;
    }

    FILE *tmp = fopen("temp.txt", "w");
    if (!tmp) {
        fclose(fp);
        printf("Error: could not create temporary file.\n");
        return;
    }

    int searchId, found = 0;
    printf("\nEnter the ID of the user to update: ");
    scanf("%d", &searchId);

    User u;
    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        if (u.id == searchId) {
            found = 1;
            printf("Updating user with ID %d\n", u.id);
            printf("Enter the new name: ");
            scanf("%s", u.name);
            printf("Enter the new age: ");
            scanf("%d", &u.age);
        }
        fprintf(tmp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(tmp);
    remove(DATA_FILE);
    rename("temp.txt", DATA_FILE);

    if (found) {
        printf("User has been updated succesfully.\n");
    } else {
        printf("There is no such user with ID %d.\n", searchId);
    }
}

// To delete a existing user
void delete() {
    FILE *fp = fopen(DATA_FILE, "r");
    if (!fp) {
        printf("No data file found.\n");
        return;
    }

    FILE *tmp = fopen("temp.txt", "w");
    if (!tmp) {
        fclose(fp);
        printf("Error: could not create temporary file.\n");
        return;
    }

    int searchId, found = 0;
    printf("\nEnter the ID of the user to delete from the file: ");
    scanf("%d", &searchId);

    User u;
    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) == 3) {
        if (u.id == searchId) {
            found = 1;
            continue; // To skip this record
        }
        fprintf(tmp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(tmp);
    remove(DATA_FILE);
    rename("temp.txt", DATA_FILE);

    if (found) {
        printf("User has been deleted successfully\n");
    } else {
        printf("No such user exist with ID %d.\n", searchId);
    }
}

int main() {
    int choice;

    while (1) {
        printf("User Management System\n");
        printf("\n");
        printf("1. Add User\n");
        printf("2. Show All Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: create(); break;
            case 2: display(); break;
            case 3: update(); break;
            case 4: delete(); break;
            case 5:
                printf("To exit the program\n");
                return 0;
            default:
                printf("The choice is Invalid. Please try again.\n");
        }
    }
}
