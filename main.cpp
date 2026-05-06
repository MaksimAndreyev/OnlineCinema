#include <iostream>
#include <limits>
#include "classes.h"

const std::string ADMIN_PASSWORD = "admin123";

Catalog catalog;
std::vector<User> users;
std::vector<Review> reviews;
User* currentUser = nullptr;
Administrator admin;

void clearScreen() {}

void waitForEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int readInt(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (!std::cin.fail() && value >= min && value <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Error! Enter a number from " << min << " to " << max << ".\n";
    }
}

void mainMenu();
void userMenu();
void adminMenu();

void registerUser();
void loginUser();
void searchFilms();
void showCatalog();
void watchFilm();
void buySubscription();
void rateFilm();

void addFilm();
void removeFilm();
void changeFilmAccess();

// ==================== main ====================

int main() {
    std::cout << "Welcome to the online cinema!\n";
    mainMenu();
    return 0;
}

// ==================== Menus ====================

void mainMenu() {
    while (true) {
        std::cout << "\n=== MAIN MENU ===\n";
        std::cout << "1. View catalog\n";
        std::cout << "2. Search film by title\n";
        std::cout << "3. Log in as user\n";
        std::cout << "4. Log in as administrator\n";
        std::cout << "5. Exit\n";

        int choice = readInt("Choose an action: ", 1, 5);

        switch (choice) {
        case 1:
            showCatalog();
            break;
        case 2:
            searchFilms();
            break;
        case 3:
            if (currentUser == nullptr) {
                std::cout << "1. Register\n";
                std::cout << "2. Log in\n";
                int userChoice = readInt("Choose an action: ", 1, 2);
                if (userChoice == 1) {
                    registerUser();
                }
                else {
                    loginUser();
                }
            }
            if (currentUser != nullptr) {
                userMenu();
            }
            break;
        case 4: {
            std::string password;
            std::cout << "Enter administrator password: ";
            std::getline(std::cin, password);
            if (password == ADMIN_PASSWORD) {
                std::cout << "Access granted.\n";
                adminMenu();
            }
            else {
                std::cout << "Incorrect password!\n";
                waitForEnter();
            }
            break;
        }
        case 5:
            std::cout << "Goodbye!\n";
            return;
        }
    }
}

void userMenu() {
    while (currentUser != nullptr) {
        std::cout << "\n=== USER MENU [" << currentUser->getLogin() << "] ===\n";
        std::cout << "1. View catalog\n";
        std::cout << "2. Search film by title\n";
        std::cout << "3. Watch a film\n";
        std::cout << "4. Buy subscription\n";
        std::cout << "5. Rate a film\n";
        std::cout << "6. Log out\n";

        int choice = readInt("Choose an action: ", 1, 6);

        switch (choice) {
        case 1: showCatalog(); break;
        case 2: searchFilms(); break;
        case 3: watchFilm(); break;
        case 4: buySubscription(); break;
        case 5: rateFilm(); break;
        case 6:
            currentUser->logout();
            currentUser = nullptr;
            std::cout << "You have logged out.\n";
            break;
        }
    }
}

void adminMenu() {
    while (true) {
        std::cout << "\n=== ADMIN MENU ===\n";
        std::cout << "1. View catalog\n";
        std::cout << "2. Search film by title\n";
        std::cout << "3. Add film\n";
        std::cout << "4. Remove film\n";
        std::cout << "5. Change film access\n";
        std::cout << "6. Back to main menu\n";

        int choice = readInt("Choose an action: ", 1, 6);

        switch (choice) {
        case 1: showCatalog(); break;
        case 2: searchFilms(); break;
        case 3: addFilm(); break;
        case 4: removeFilm(); break;
        case 5: changeFilmAccess(); break;
        case 6: return;
        }
    }
}

// ==================== User actions ====================

void registerUser() {
    std::string login, password;
    std::cout << "Enter login: ";
    std::getline(std::cin, login);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    for (const auto& user : users) {
        if (user.getLogin() == login) {
            std::cout << "A user with this login already exists!\n";
            return;
        }
    }

    users.emplace_back();
    users.back().registerUser(login, password);
    currentUser = &users.back();
    std::cout << "Registration successful! You are now logged in.\n";
}

void loginUser() {
    std::string login, password;
    std::cout << "Enter login: ";
    std::getline(std::cin, login);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    for (auto& user : users) {
        user.loginUser(login, password);
        if (user.isAuthorized()) {
            currentUser = &user;
            std::cout << "Login successful!\n";
            return;
        }
    }
    std::cout << "Invalid login or password.\n";
}

void searchFilms() {
    std::string title;
    std::cout << "Enter film title to search: ";
    std::getline(std::cin, title);

    auto found = catalog.findFilms(title);
    if (found.empty()) {
        std::cout << "No films found with that title.\n";
    }
    else {
        std::cout << "\nFound " << found.size() << " film(s):\n";
        for (size_t i = 0; i < found.size(); ++i) {
            std::cout << i + 1 << ". ID: " << found[i]->getId()
                << " | " << found[i]->getTitle()
                << " | Genre: " << found[i]->getGenre()
                << " | Year: " << found[i]->getYear()
                << " | Rating: " << found[i]->getRating()
                << " | Access: " << (found[i]->isAccessible() ? "open" : "subscription")
                << "\n";
        }
    }
    waitForEnter();
}

void showCatalog() {
    std::cout << "\n=== FILM CATALOG ===\n";
    auto& films = catalog.getAllFilms();
    if (films.empty()) {
        std::cout << "The catalog is empty.\n";
        return;
    }
    for (const auto& film : films) {
        std::cout << "ID: " << film.getId()
            << " | " << film.getTitle()
            << " | Genre: " << film.getGenre()
            << " | Year: " << film.getYear()
            << " | Rating: " << film.getRating()
            << " | Access: " << (film.isAccessible() ? "open" : "subscription")
            << "\n";
    }
    waitForEnter();
}

void watchFilm() {
    std::string title;
    std::cout << "Enter film title: ";
    std::getline(std::cin, title);

    auto found = catalog.findFilms(title);
    if (found.empty()) {
        std::cout << "Film not found.\n";
        return;
    }

    if (found.size() > 1) {
        std::cout << "\nSeveral films found with this title:\n";
        for (size_t i = 0; i < found.size(); ++i) {
            std::cout << i + 1 << ". ID: " << found[i]->getId()
                << " | Genre: " << found[i]->getGenre()
                << " | Year: " << found[i]->getYear()
                << " | Rating: " << found[i]->getRating()
                << " | Access: " << (found[i]->isAccessible() ? "open" : "subscription")
                << "\n";
        }
        int choice = readInt("Choose a film number: ", 1, (int)found.size());
        Film* selected = found[choice - 1];

        if (selected->isAccessible() || currentUser->hasSubscription()) {
            std::cout << "Now watching: " << selected->getTitle() << "\n";
            std::cout << "Metadata: " << selected->getMetadata() << "\n";
            std::cout << "Enjoy!";
            if (!selected->isAccessible()) std::cout << " (subscription access)";
            std::cout << "\n";
        }
        else {
            std::cout << "Film is unavailable. Subscription required.\n";
        }
    }
    else {
        Film* film = found[0];
        if (film->isAccessible() || currentUser->hasSubscription()) {
            std::cout << "Now watching: " << film->getTitle() << "\n";
            std::cout << "Metadata: " << film->getMetadata() << "\n";
            std::cout << "Enjoy!";
            if (!film->isAccessible()) std::cout << " (subscription access)";
            std::cout << "\n";
        }
        else {
            std::cout << "Film is unavailable. Subscription required.\n";
        }
    }
    waitForEnter();
}

void buySubscription() {
    std::cout << "\n=== BUY SUBSCRIPTION ===\n";
    std::cout << "1. 1 month (30 days)\n";
    std::cout << "2. 6 months (180 days)\n";
    std::cout << "3. 1 year (365 days)\n";

    int choice = readInt("Choose a period: ", 1, 3);
    int months;

    switch (choice) {
    case 1: months = 1; break;
    case 2: months = 6; break;
    case 3: months = 12; break;
    }

    currentUser->buySubscription(months);
    std::cout << "Subscription purchased";
    switch (choice) {
    case 1: std::cout << " for 1 month!\n"; break;
    case 2: std::cout << " for 6 months!\n"; break;
    case 3: std::cout << " for 1 year!\n"; break;
    }
    waitForEnter();
}

void rateFilm() {
    std::string title;
    std::cout << "Enter film title: ";
    std::getline(std::cin, title);

    auto found = catalog.findFilms(title);
    if (found.empty()) {
        std::cout << "Film not found.\n";
        return;
    }

    Film* selected = nullptr;

    if (found.size() > 1) {
        std::cout << "\nSeveral films found with this title:\n";
        for (size_t i = 0; i < found.size(); ++i) {
            std::cout << i + 1 << ". ID: " << found[i]->getId()
                << " | Genre: " << found[i]->getGenre()
                << " | Year: " << found[i]->getYear()
                << " | Rating: " << found[i]->getRating() << "\n";
        }
        int choice = readInt("Choose a film number: ", 1, (int)found.size());
        selected = found[choice - 1];
    }
    else {
        selected = found[0];
    }

    // Проверяем, не оставлял ли уже пользователь отзыв к этому фильму
    for (const auto& review : reviews) {
        if (review.getFilmId() == selected->getId() &&
            review.getAuthorLogin() == currentUser->getLogin()) {
            std::cout << "You have already reviewed this film!\n";
            return;
        }
    }

    int score = readInt("Enter your rating (1-10): ", 1, 10);

    std::string text;
    std::cout << "Enter review text (max 500 characters, or press Enter to skip): ";
    std::getline(std::cin, text);

    // Обрезаем текст до 500 символов, если длиннее
    if (text.length() > 500) {
        text = text.substr(0, 500);
        std::cout << "Review text has been trimmed to 500 characters.\n";
    }

    currentUser->rateFilm(catalog, reviews, selected->getTitle(), selected->getId(), score, text);
    std::cout << "Review added! Film rating updated to " << selected->getRating() << "\n";
    waitForEnter();
}

// ==================== Admin actions ====================

void addFilm() {
    std::string title, genre, metadata;
    int year;

    std::cout << "Enter title: ";
    std::getline(std::cin, title);
    std::cout << "Enter genre: ";
    std::getline(std::cin, genre);
    year = readInt("Enter release year: ", 1900, 2026);
    std::cout << "Enter description (metadata): ";
    std::getline(std::cin, metadata);

    admin.addFilm(catalog, title, genre, year, metadata);
    std::cout << "Film added! ID: " << catalog.getAllFilms().size() << "\n";
    waitForEnter();
}

void removeFilm() {
    std::string title;
    std::cout << "Enter film title to remove: ";
    std::getline(std::cin, title);

    auto found = catalog.findFilms(title);
    if (found.empty()) {
        std::cout << "Film not found.\n";
        return;
    }

    if (found.size() > 1) {
        std::cout << "\nSeveral films found with this title:\n";
        for (size_t i = 0; i < found.size(); ++i) {
            std::cout << i + 1 << ". ID: " << found[i]->getId()
                << " | Genre: " << found[i]->getGenre()
                << " | Year: " << found[i]->getYear()
                << " | Rating: " << found[i]->getRating() << "\n";
        }
        int choice = readInt("Choose a film number to remove: ", 1, (int)found.size());
        int idToRemove = found[choice - 1]->getId();
        admin.removeFilm(catalog, title, idToRemove);
        std::cout << "Film removed.\n";
    }
    else {
        int idToRemove = found[0]->getId();
        admin.removeFilm(catalog, title, idToRemove);
        std::cout << "Film \"" << found[0]->getTitle() << "\" removed.\n";
    }
    waitForEnter();
}

void changeFilmAccess() {
    std::string title;
    std::cout << "Enter film title: ";
    std::getline(std::cin, title);

    auto found = catalog.findFilms(title);
    if (found.empty()) {
        std::cout << "Film not found.\n";
        return;
    }

    int idToChange;
    std::string filmTitle;

    if (found.size() > 1) {
        std::cout << "\nSeveral films found with this title:\n";
        for (size_t i = 0; i < found.size(); ++i) {
            std::cout << i + 1 << ". ID: " << found[i]->getId()
                << " | Genre: " << found[i]->getGenre()
                << " | Year: " << found[i]->getYear()
                << " | Access: " << (found[i]->isAccessible() ? "open" : "subscription")
                << "\n";
        }
        int choice = readInt("Choose a film number: ", 1, (int)found.size());
        idToChange = found[choice - 1]->getId();
        filmTitle = found[choice - 1]->getTitle();
    }
    else {
        idToChange = found[0]->getId();
        filmTitle = found[0]->getTitle();
    }

    int choice = readInt("Make film accessible to everyone? (1 - yes, 0 - no): ", 0, 1);
    admin.changeFilmAccess(catalog, title, idToChange, choice == 1);
    std::cout << "Access to \"" << filmTitle
        << "\" (ID: " << idToChange << ") has been changed.\n";
    waitForEnter();
}