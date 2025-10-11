#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// To check it's a valid operator or not
int validOperator(char character) {
    return (character == '+' || character == '-' || character == '*' || character == '/');
}

// To follow the precedence
int preced(char operators) {
    if (operators == '+' || operators == '-') return 1;
    if (operators == '*' || operators == '/') return 2;
    return 0;
}

// To calculate the operation
int applyOp(int operatorOne, int operatorTwo, char operators, int *error) {
    switch (operators) {
        case '+': return operatorOne + operatorTwo;
        case '-': return operatorOne - operatorTwo;
        case '*': return operatorOne * operatorTwo;
        case '/': 
            if (operatorTwo == 0) {
                *error = 1; // if divison by zero
                return 0;
            }
            return operatorOne / operatorTwo;  // if divison is by non zero
    }
    return 0;
}

// To evaluate the operation
int evaluate(char *tokens, int *error) {
    int position;
    int values[100], valueTop = -1; // Stack of value
    char operatorStack[100]; int operatorTop = -1; // Stack of operator

    for (position = 0; position < strlen(tokens); position++) {
        if (tokens[position] == ' ') continue; // Condition to ignore whitespaces

        // If it is a number then only it will be parsed
        if (isdigit(tokens[position])) {
            int value = 0;
            while (position < strlen(tokens) && isdigit(tokens[position])) {
                value = (value * 10) + (tokens[position] - '0');
                position++;
            }
            values[++valueTop] = value;
            position--; // rollback one step
        }
        // If the character is invalid
        else if (!validOperator(tokens[position])) {
            *error = 2; // Invalid character
            return 0;
        }
        // If the operator is encountered
        else {
            while (operatorTop != -1 && preced(operatorStack[operatorTop]) >= preced(tokens[position])) {
                int operatorTwo = values[valueTop--];
                int operatorOne = values[valueTop--];
                char operators = operatorStack[operatorTop--];
                int result = applyOp(operatorOne, operatorTwo, operators, error);
                if (*error == 1) return 0; // Division by zero
                values[++valueTop] = result;
            }
            operatorStack[++operatorTop] = tokens[position];
        }
    }

    // Apply remaining ops
    while (operatorTop != -1) {
        int operatorTwo = values[valueTop--];
        int operatorOne = values[valueTop--];
        char operators = operatorStack[operatorTop--];
        int result = applyOp(operatorOne, operatorTwo, operators, error);
        if (*error == 1) return 0;
        values[++valueTop] = result;
    }

    return values[valueTop];
}

int main() {
    char expression[200];
    printf("Enter expression: ");
    fgets(expression, sizeof(expression), stdin);

    // to remove any trailing line
    expression[strcspn(expression, "\n")] = 0;

    int errorFlag = 0;
    int result = evaluate(expression, &errorFlag);

    if (errorFlag == 1)
        printf("Error: Division by zero.\n");
    else if (errorFlag == 2)
        printf("Error: Invalid expression.\n");
    else
        printf("%d\n", result);

    return 0;
}
