#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Players_data.h"

#define MAX_TEAMS 10
#define MIN_PLAYER_ID 1
#define MAX_PLAYER_ID 1000
#define MIN_PLAYERS_PER_TEAM 11
#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LEN 50


//Defined a new dataype for role of the player
typedef enum {
    RoleBatsman = 1,     
    RoleBowler  = 2,     
    RoleAllRounder = 3  
} PlayerRole;


// Struct for Player Node
typedef struct PlayerRecord {
    int playerId;                      
    char playerName[51];               
    char teamName[31];                 
    PlayerRole role;                   
    int totalRuns;                     
    float battingAverage;              
    float strikeRate;                  
    int wickets;                       
    float economyRate;                 
    float performanceIndex;            
    struct PlayerRecord* next;         
} PlayerRecord;


// Struct for team information 
typedef struct TeamInfo {
    int teamId;                        
    char teamName[31];                 
    int totalPlayers;                  
    float avgBattingStrikeRate;        
} TeamInfo;


// to declare global pointers
static PlayerRecord* headRecord = NULL;   // head node of linked list of players
static TeamInfo teamSummary[20];          // stores one entry per team


// As data is storing roles as string 
static PlayerRole roleFromString(const char* roleText) {
    if (strcmp(roleText, "Batsman") == 0) {
        return RoleBatsman;
    }
    if (strcmp(roleText, "Bowler") == 0) {
        return RoleBowler;
    }
    return RoleAllRounder;
}


// Calculating the Performance Index according to the requirement
static float computePerformanceIndex(const PlayerRecord* node) {
    if (node == NULL) {
        return 0;
    }
    if (node->role == RoleBatsman) {
        float value = (node->battingAverage * node->strikeRate) / 100;
        return value;
    }
    else if (node->role == RoleBowler) {
        float value = (node->wickets * 2) + (100 - node->economyRate);
        return value;
    }
    else {
        float value = ((node->battingAverage * node->strikeRate) / 100) + (node->wickets * 2);
        return value;
    }
}


// To add a new user we are adding a new node to it
static void appendRecord(PlayerRecord* newRecord) {
    newRecord->next = NULL;
    if (headRecord == NULL) {
        headRecord = newRecord;
        return;
    }
    PlayerRecord* walker = headRecord;
    while (walker->next != NULL) {
        walker = walker->next;
    }
    walker->next = newRecord;
}


// To find the team ID
static TeamInfo* findTeamById(int teamId) {
    if (teamId < 1) {
        return NULL;
    }

    if (teamCount < 1) {
        return NULL;
    }

    if (teamId > teamCount) {
        return NULL;
    }

    TeamInfo* ptr = teamSummary;    // pointer to first TeamInfo
    int currentId = 1;

    while (currentId < teamId) {
        ptr++;
        currentId++;
    }

    return ptr;
}


// To find team index by name
static int findTeamIndexByName(const char* teamName) {
    int index;
    for (index = 0; index < teamCount; index++) {
        if (strcmp(teams[index], teamName) == 0) {
            return index;
        }
    }
    return -1;
}


// To compare the team scores
static int compareTeamsByAvgSrDesc(const void* a, const void* b) {
    const TeamInfo* teamA = (const TeamInfo*) a;
    const TeamInfo* teamB = (const TeamInfo*) b;
    if (teamA->avgBattingStrikeRate < teamB->avgBattingStrikeRate) {
        return 1;
    }
    if (teamA->avgBattingStrikeRate > teamB->avgBattingStrikeRate) {
        return -1;
    }
    return 0;
}


//To arrange the record by performance index in descending order
static int comparePlayerPointersByPerfDesc(const void* a, const void* b) {
    const PlayerRecord* playerA = *(const PlayerRecord**) a;
    const PlayerRecord* playerB = *(const PlayerRecord**) b;
    if (playerA->performanceIndex < playerB->performanceIndex) {
        return 1;
    }
    if (playerA->performanceIndex > playerB->performanceIndex) {
        return -1;
    }
    return 0;
}


// It builds the entire linked list of players and calculates all team statistics (players count + avg strike rate) from the header data
void initializeData() {
    int index;

    TeamInfo* teamPtr = teamSummary;
    for (index = 0; index < teamCount; index++) {
        teamPtr->teamId = index + 1;
        strncpy(teamPtr->teamName, teams[index], sizeof(teamPtr->teamName) - 1);
        teamPtr->teamName[sizeof(teamPtr->teamName) - 1] = '\0';
        teamPtr->totalPlayers = 0;
        teamPtr->avgBattingStrikeRate = 0.0f;
        teamPtr++;
    }

    float* strikeRateSum = (float*) calloc((size_t) teamCount, sizeof(float));
    int* strikeRateCount = (int*) calloc((size_t) teamCount, sizeof(int));

    if (strikeRateSum == NULL || strikeRateCount == NULL) {
        printf("Memory allocation failed\n");
        if (strikeRateSum != NULL) {
            free(strikeRateSum);
        }
        if (strikeRateCount != NULL) {
            free(strikeRateCount);
        }
        exit(1);
    }

    typeof(players[0]) * playerPtr = players;
    typeof(players[0]) * playersEnd = players + playerCount;

    for (; playerPtr < playersEnd; playerPtr++) {
        PlayerRecord* newNode = (PlayerRecord*) malloc(sizeof(PlayerRecord));

        if (newNode == NULL) {
            printf("Memory allocation failed\n");
            free(strikeRateSum);
            free(strikeRateCount);
            exit(1);
        }

        newNode->playerId = playerPtr->id;
        strncpy(newNode->playerName, playerPtr->name, sizeof(newNode->playerName) - 1);
        newNode->playerName[sizeof(newNode->playerName) - 1] = '\0';
        strncpy(newNode->teamName, playerPtr->team, sizeof(newNode->teamName) - 1);
        newNode->teamName[sizeof(newNode->teamName) - 1] = '\0';
        newNode->role = roleFromString(playerPtr->role);
        newNode->totalRuns = playerPtr->totalRuns;
        newNode->battingAverage = playerPtr->battingAverage;
        newNode->strikeRate = playerPtr->strikeRate;
        newNode->wickets = playerPtr->wickets;
        newNode->economyRate = playerPtr->economyRate;

        newNode->performanceIndex = computePerformanceIndex(newNode);

        appendRecord(newNode);

        int teamIdx = findTeamIndexByName(playerPtr->team);

        if (teamIdx != -1) {
            TeamInfo* teamUpdatePtr = teamSummary + teamIdx;
            teamUpdatePtr->totalPlayers += 1;

            if (newNode->role == RoleBatsman || newNode->role == RoleAllRounder) {
                float* sumPtr = strikeRateSum + teamIdx;
                int* countPtr = strikeRateCount + teamIdx;
                *sumPtr = *sumPtr + newNode->strikeRate;
                *countPtr = *countPtr + 1;
            }
        }
    }

    TeamInfo* finalizePtr = teamSummary;
    float* sumWalker = strikeRateSum;
    int* countWalker = strikeRateCount;

    for (index = 0; index < teamCount; index++) {
        if (*countWalker > 0) {
            finalizePtr->avgBattingStrikeRate = (*sumWalker) / ((float) *countWalker);
        }
        else {
            finalizePtr->avgBattingStrikeRate = 0.0f;
        }
        finalizePtr++;
        sumWalker++;
        countWalker++;
    }

    free(strikeRateSum);
    free(strikeRateCount);
}

// Case 1: To add players in the existing header file Player_Data.h
static void addPlayerToTeamCase() {
    printf("Available teams:\n");
    TeamInfo* tempPtr = teamSummary;
    int listIndex;
    for (listIndex = 0; listIndex < teamCount; listIndex++) {
        printf("  %d -> %s (players: %d)\n", tempPtr->teamId, tempPtr->teamName, tempPtr->totalPlayers);
        tempPtr++;
    }

    int teamIdInput = 0;
    printf("Enter Team ID to add player (1-%d): ", teamCount);

    if (scanf("%d", &teamIdInput) != 1) {
        while (getchar() != '\n') { }
        printf("Invalid team id input.\n");
        return;
    }

    if (teamIdInput < 1 || teamIdInput > teamCount) {
        printf("Team ID %d out of range (1..%d).\n", teamIdInput, teamCount);
        return;
    }

    TeamInfo* chosenTeam = findTeamById(teamIdInput);
    if (chosenTeam == NULL) {
        printf("Team ID %d could not be found.\n", teamIdInput);
        return;
    }

    if (chosenTeam->totalPlayers >= MAX_PLAYERS_PER_TEAM) {
        printf("Team %s already has %d players. Cannot add more.\n",
               chosenTeam->teamName, MAX_PLAYERS_PER_TEAM);
        return;
    }

    while (getchar() != '\n') { }

    PlayerRecord* newNode = malloc(sizeof(PlayerRecord));
    if (newNode == NULL) {
        printf("Allocation failed.\n");
        return;
    }

    printf("Enter Player ID: ");
    if (scanf("%d", &newNode->playerId) != 1) {
        while (getchar() != '\n') { }
        printf("Invalid Player ID.\n");
        free(newNode);
        return;
    }

    if (newNode->playerId < MIN_PLAYER_ID || newNode->playerId > MAX_PLAYER_ID) {
        printf("Player ID must be between %d and %d.\n", MIN_PLAYER_ID, MAX_PLAYER_ID);
        free(newNode);
        return;
    }

    PlayerRecord* checkPtr = headRecord;
    while (checkPtr != NULL) {
        if (checkPtr->playerId == newNode->playerId) {
            printf("Player ID %d already exists for '%s'.\n",
                   newNode->playerId, checkPtr->playerName);
            free(newNode);
            return;
        }
        checkPtr = checkPtr->next;
    }

    while (getchar() != '\n') { }

    printf("Enter Name: ");
    if (fgets(newNode->playerName, sizeof(newNode->playerName), stdin) == NULL) {
        strncpy(newNode->playerName, "Unknown Player", sizeof(newNode->playerName));
    }

    size_t nameLen = strlen(newNode->playerName);
    if (nameLen > 0 && newNode->playerName[nameLen - 1] == '\n') {
        newNode->playerName[nameLen - 1] = '\0';
        nameLen--;
    }

    if (nameLen < 1 || nameLen > MAX_NAME_LEN) {
        printf("Name must be 1..%d characters.\n", MAX_NAME_LEN);
        free(newNode);
        return;
    }

    int roleInput = 0;
    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &roleInput) != 1) {
        while (getchar() != '\n') { }
        printf("Invalid role.\n");
        free(newNode);
        return;
    }

    if (roleInput < 1 || roleInput > 3) {
        printf("Role must be 1..3.\n");
        free(newNode);
        return;
    }

    newNode->role = (PlayerRole) roleInput;

    printf("Total Runs: ");
    if (scanf("%d", &newNode->totalRuns) != 1) {
        newNode->totalRuns = 0;
        while (getchar() != '\n') { }
    }

    printf("Batting Average: ");
    if (scanf("%f", &newNode->battingAverage) != 1) {
        newNode->battingAverage = 0.0;
        while (getchar() != '\n') { }
    }

    printf("Strike Rate: ");
    if (scanf("%f", &newNode->strikeRate) != 1) {
        newNode->strikeRate = 0.0;
        while (getchar() != '\n') { }
    }

    printf("Wickets: ");
    if (scanf("%d", &newNode->wickets) != 1) {
        newNode->wickets = 0;
        while (getchar() != '\n') { }
    }

    printf("Economy Rate: ");
    if (scanf("%f", &newNode->economyRate) != 1) {
        newNode->economyRate = 0.0;
        while (getchar() != '\n') { }
    }

    strncpy(newNode->teamName, chosenTeam->teamName, sizeof(newNode->teamName) - 1);
    newNode->teamName[sizeof(newNode->teamName) - 1] = '\0';

    newNode->performanceIndex = computePerformanceIndex(newNode);

    appendRecord(newNode);

    chosenTeam->totalPlayers++;

    float srSum = 0.0;
    int srCount = 0;
    PlayerRecord* walker = headRecord;

    while (walker != NULL) {
        if (strcmp(walker->teamName, chosenTeam->teamName) == 0) {
            if (walker->role == RoleBatsman || walker->role == RoleAllRounder) {
                srSum += walker->strikeRate;
                srCount++;
            }
        }
        walker = walker->next;
    }

    if (srCount > 0) {
        chosenTeam->avgBattingStrikeRate = srSum / srCount;
    }
    else {
        chosenTeam->avgBattingStrikeRate = 0.0;
    }

    printf("Player added successfully to Team %s!\n", chosenTeam->teamName);
}



// Case 2: To display the players of a specific team
static void displayPlayersOfTeamCase() {
    int teamIdInput = 0;
    printf("Enter Team ID: ");
    if (scanf("%d", &teamIdInput) != 1) {
        while (getchar() != '\n') { }
        printf("Invalid team id input.\n");
        return;
    }

    TeamInfo* chosenTeam = findTeamById(teamIdInput);
    if (chosenTeam == NULL) {
        printf("Team ID not found.\n");
        return;
    }

    printf("\nPlayers of Team %s:\n", chosenTeam->teamName);
    printf("====================================================================================\n");
    printf("ID\tName                     \tRole           \tRuns\tAvg\tSR\tWkts\tER\tPerf.Index\n");
    printf("====================================================================================\n");

    PlayerRecord* walker = headRecord;
    while (walker != NULL) {
        if (strcmp(walker->teamName, chosenTeam->teamName) == 0) {
            const char* roleText = (walker->role == RoleBatsman) ? "Batsman" : (walker->role == RoleBowler) ? "Bowler" : "All-rounder";
            printf("%d\t%-25s\t%-15s\t%d\t%.1f\t%.1f\t%d\t%.1f\t%.2f\n",
                   walker->playerId, walker->playerName, roleText,
                   walker->totalRuns, walker->battingAverage, walker->strikeRate,
                   walker->wickets, walker->economyRate, walker->performanceIndex);
        }
        walker = walker->next;
    }

    printf("====================================================================================\n");
    printf("Total Players: %d\n", chosenTeam->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", chosenTeam->avgBattingStrikeRate);
}


// Case 3: To display the team by average strike rate
static void displayTeamsByAvgSrCase() {
    TeamInfo tempArray[20];
    int index;
    for (index = 0; index < teamCount; index++) {
        tempArray[index] = teamSummary[index];
    }

    qsort(tempArray, (size_t) teamCount, sizeof(TeamInfo), compareTeamsByAvgSrDesc);

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("=========================================================\n");
    printf("ID\tTeam Name       \tAvg Bat SR\tTotal Players\n");
    printf("=========================================================\n");

    for (index = 0; index < teamCount; index++) {
        printf("%d\t%-15s\t%.2f\t\t%d\n",
               tempArray[index].teamId,
               tempArray[index].teamName,
               tempArray[index].avgBattingStrikeRate,
               tempArray[index].totalPlayers);
    }

    printf("=========================================================\n");
}

// Case 4: To display the top k players of a specifc team of specific role
static void displayTopKPlayersCase() {
    int teamIdInput = 0;
    printf("Enter Team ID: ");
    if (scanf("%d", &teamIdInput) != 1) {
        while (getchar() != '\n') { }
        printf("Invalid team id input.\n");
        return;
    }

    TeamInfo* chosenTeam = findTeamById(teamIdInput);
    if (chosenTeam == NULL) {
        printf("Team ID not found.\n");
        return;
    }

    int roleInput = 0;
    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &roleInput) != 1) {
        while (getchar() != '\n') { }
        printf("Invalid role input.\n");
        return;
    }

    if (roleInput < 1 || roleInput > 3) {
        printf("Role must be 1..3.\n");
        return;
    }

    PlayerRole chosenRole = (PlayerRole) roleInput;

    int kInput = 0;
    printf("Enter number of players (K): ");
    if (scanf("%d", &kInput) != 1) {
        while (getchar() != '\n') { }
        printf("Invalid K input.\n");
        return;
    }

    if (kInput <= 0) {
        printf("K must be positive.\n");
        return;
    }

    int matchCount = 0;
    PlayerRecord* walker = headRecord;
    while (walker != NULL) {
        if (strcmp(walker->teamName, chosenTeam->teamName) == 0 && walker->role == chosenRole) {
            matchCount++;
        }
        walker = walker->next;
    }

    if (matchCount == 0) {
        printf("No players found for Team %s with chosen role.\n", chosenTeam->teamName);
        return;
    }

    PlayerRecord** pointerArray = (PlayerRecord**) malloc(sizeof(PlayerRecord*) * matchCount);
    if (pointerArray == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    int fillIndex = 0;
    walker = headRecord;
    while (walker != NULL) {
        if (strcmp(walker->teamName, chosenTeam->teamName) == 0 && walker->role == chosenRole) {
            pointerArray[fillIndex++] = walker;
        }
        walker = walker->next;
    }

    qsort(pointerArray, (size_t) matchCount, sizeof(PlayerRecord*), comparePlayerPointersByPerfDesc);

    int printCount = (kInput < matchCount) ? kInput : matchCount;

    const char* roleText = (chosenRole == RoleBatsman) ? "Batsman" : (chosenRole == RoleBowler) ? "Bowler" : "All-rounder";
    printf("\nTop %d %s players of Team %s:\n", printCount, roleText, chosenTeam->teamName);
    printf("====================================================================================\n");
    printf("ID\tName                     \tRole           \tRuns\tAvg\tSR\tWkts\tER\tPerf.Index\n");
    printf("====================================================================================\n");

    int printIndex;
    for (printIndex = 0; printIndex < printCount; printIndex++) {
        PlayerRecord* nodePtr = pointerArray[printIndex];
        printf("%d\t%-25s\t%-15s\t%d\t%.1f\t%.1f\t%d\t%.1f\t%.2f\n",
               nodePtr->playerId, nodePtr->playerName,
               roleText,
               nodePtr->totalRuns, nodePtr->battingAverage, nodePtr->strikeRate,
               nodePtr->wickets, nodePtr->economyRate, nodePtr->performanceIndex);
    }

    printf("====================================================================================\n");
    free(pointerArray);
}


// Case 5: to display the players by their role 
static void displayPlayersByRoleCase() {
    int roleInput = 0;
    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &roleInput) != 1) {
        while (getchar() != '\n') {
            /* flush */
        }
        printf("Invalid input.\n");
        return;
    }

    if (roleInput < 1 || roleInput > 3) {
        printf("Role must be 1..3.\n");
        return;
    }

    PlayerRole chosenRole = (PlayerRole) roleInput;

    int totalOfRole = 0;
    PlayerRecord* walker = headRecord;
    while (walker != NULL) {
        if (walker->role == chosenRole) {
            totalOfRole++;
        }
        walker = walker->next;
    }

    if (totalOfRole == 0) {
        printf("No players found for selected role.\n");
        return;
    }

    PlayerRecord** pointerArr = (PlayerRecord**) malloc(sizeof(PlayerRecord*) * totalOfRole);
    if (pointerArr == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    int fillIndex = 0;
    walker = headRecord;
    while (walker != NULL) {
        if (walker->role == chosenRole) {
            pointerArr[fillIndex++] = walker;
        }
        walker = walker->next;
    }

    qsort(pointerArr, (size_t) totalOfRole, sizeof(PlayerRecord*), comparePlayerPointersByPerfDesc);

    const char* roleText = (chosenRole == RoleBatsman) ? "Batsman" : (chosenRole == RoleBowler) ? "Bowler" : "All-rounder";
    printf("\nAll %s players across all teams by Performance Index (desc):\n", roleText);
    printf("======================================================================================\n");
    printf("ID\tName                     \tTeam           \tRole           \tRuns\tAvg\tSR\tWkts\tER\tPerf.Index\n");
    printf("======================================================================================\n");

    int printIndex;
    for (printIndex = 0; printIndex < totalOfRole; printIndex++) {
        PlayerRecord* nodePtr = pointerArr[printIndex];
        printf("%d\t%-25s\t%-15s\t%-15s\t%d\t%.1f\t%.1f\t%d\t%.1f\t%.2f\n",
               nodePtr->playerId, nodePtr->playerName, nodePtr->teamName,
               roleText,
               nodePtr->totalRuns, nodePtr->battingAverage, nodePtr->strikeRate,
               nodePtr->wickets, nodePtr->economyRate, nodePtr->performanceIndex);
    }

    printf("======================================================================================\n");
    free(pointerArr);
}


// To display the menu
void displayTheMenu() {
    int userChoice = 0;
    while (1) {
        printf("\n=====================================================\n");
        printf("ICC ODI Player Performance Analyzer\n");
        printf("=====================================================\n");
        printf("1. Add Player to Team\n");
        printf("2. Display Players of a Specific Team\n");
        printf("3. Display Teams by Average Batting Strike Rate\n");
        printf("4. Display Top K Players of a Specific Team by Role\n");
        printf("5. Display all Players of specific role Across All Teams by performance index\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &userChoice) != 1) {
            while (getchar() != '\n') {
                /* flush */
            }
            printf("Invalid input. Try again.\n");
            continue;
        }

        switch (userChoice) {
            case 1: {
                addPlayerToTeamCase();
                break;
            }
            case 2: {
                displayPlayersOfTeamCase();
                break;
            }
            case 3: {
                displayTeamsByAvgSrCase();
                break;
            }
            case 4: {
                displayTopKPlayersCase();
                break;
            }
            case 5: {
                displayPlayersByRoleCase();
                break;
            }
            case 6: {
                printf("Exiting program. Goodbye!\n");
                return;
            }
            default: {
                printf("Unknown choice. Please enter 1-6.\n");
                break;
            }
        }
    }
}

// ----- end of Players_data.c -----
