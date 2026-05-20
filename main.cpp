#include <iostream>
#include <limits>
#include "classes.h"
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <cctype>

const std::string ADMIN_PASSWORD = "eea96ab5ea65bb0c";
const std::string USER_DATA_FILE = "data/userData.txt";
const std::string FILM_DATA_FILE = "data/filmsData.txt";
const std::string PURCHASE_CODES_FILE = "data/purchaseCodes.txt";

const std::vector<std::string> GENRES = {
    "Action",
    "Comedy",
    "Drama",
    "Horror",
    "Science Fiction",
    "Thriller",
    "Romance",
    "Documentary",
    "Animation",
    "Adventure"
};

Catalog catalog;
std::vector<User> users;
std::vector<Review> reviews;
User* currentUser = nullptr;
Administrator admin;

std::string hashString(const std::string& input) {
    std::hash<std::string> hasher;
    size_t hashValue = hasher(input);

    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hashValue;
    return ss.str();
}

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

std::string toLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = std::tolower(c);
    }
    return result;
}

void loadUsers() {
    std::ifstream file(USER_DATA_FILE);
    if (!file.is_open()) return;

    std::string login, passwordHash;
    std::time_t subEnd;

    while (file >> login >> passwordHash >> subEnd) {
        users.emplace_back();
        users.back().registerUser(login, passwordHash);
        users.back().setSubscriptionEnd(subEnd);
        users.back().logout();
    }
    file.close();
}

void saveUsers() {
    std::ofstream file(USER_DATA_FILE);
    if (!file.is_open()) return;

    for (const auto& user : users) {
        file << user.getLogin() << " "
            << user.getPasswordHash() << " "
            << user.getSubscriptionEnd() << "\n";
    }
    file.close();
}

void loadFilms() {
    std::ifstream file(FILM_DATA_FILE);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, '|')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 8) {
            int id = std::stoi(tokens[0]);
            std::string title = tokens[1];
            std::string genre = tokens[2];
            int year = std::stoi(tokens[3]);
            double rating = std::stod(tokens[4]);
            bool accessible = (std::stoi(tokens[5]) == 1);
            std::string isan = tokens[6];
            std::string metadata = tokens[7];

            Film film(id, title, genre, year, metadata, isan);
            film.setAccessible(accessible);
            film.updateRating(rating);
            catalog.addFilm(film);
        }
    }
    file.close();
}

void saveFilms() {
    std::ofstream file(FILM_DATA_FILE);
    if (!file.is_open()) return;

    for (const auto& film : catalog.getAllFilms()) {
        file << film.getId() << "|"
            << film.getTitle() << "|"
            << film.getGenre() << "|"
            << film.getYear() << "|"
            << film.getRating() << "|"
            << (film.isAccessible() ? "1" : "0") << "|"
            << film.getIsan() << "|"
            << film.getMetadata() << "\n";
    }
    file.close();
}

std::vector<std::pair<std::string, int>> loadPurchaseCodes() {
    std::vector<std::pair<std::string, int>> codes;
    std::ifstream file(PURCHASE_CODES_FILE);
    if (!file.is_open()) return codes;

    std::string hash;
    int durationType;
    while (file >> hash >> durationType) {
        codes.emplace_back(hash, durationType);
    }
    file.close();
    return codes;
}

void savePurchaseCodes(const std::vector<std::pair<std::string, int>>& codes) {
    std::ofstream file(PURCHASE_CODES_FILE);
    if (!file.is_open()) return;

    for (const auto& code : codes) {
        file << code.first << " " << code.second << "\n";
    }
    file.close();
}

void mainMenu();
void userMenu();
void adminMenu();

void registerUser();
void loginUser();
void displaySearchResults(const std::vector<Film*>& found);
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
    std::filesystem::create_directory("data");
    std::filesystem::create_directory("users");
    std::filesystem::create_directory("films");
    loadUsers();
    loadFilms();
    std::cout << "Welcome to the online cinema!\n";
    mainMenu();
    return 0;
}

// ==================== Menus ====================

void mainMenu() {
    while (true) {
        std::cout << "\n=== MAIN MENU ===\n";
        std::cout << "1. View catalog\n";
        std::cout << "2. Search film\n";
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
            password = hashString(password);
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
        std::cout << "2. Search film\n";
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
        std::cout << "2. Search film\n";
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

    std::string passwordHash = hashString(password);
    users.emplace_back();
    users.back().registerUser(login, passwordHash);
    currentUser = &users.back();

    // Создаём папку пользователя
    std::string userDir = "users/" + login;
    std::filesystem::create_directory(userDir);

    saveUsers();

    std::cout << "Registration successful! You are now logged in.\n";
}

void loginUser() {
    std::string login, password;
    std::cout << "Enter login: ";
    std::getline(std::cin, login);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    std::string passwordHash = hashString(password);

    for (auto& user : users) {
        user.loginUser(login, passwordHash);
        if (user.isAuthorized()) {
            currentUser = &user;
            std::cout << "Login successful!\n";
            return;
        }
    }
    std::cout << "Invalid login or password.\n";
}

// Вспомогательная функция для вывода списка найденных фильмов
void displaySearchResults(const std::vector<Film*>& found) {
    if (found.empty()) {
        std::cout << "No films found.\n";
        return;
    }
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

void searchFilms() {
    std::cout << "\n=== SEARCH FILMS ===\n";
    std::cout << "1. Search by title\n";
    std::cout << "2. Search by genre\n";
    std::cout << "3. Search by year\n";

    int choice = readInt("Choose search type: ", 1, 3);

    switch (choice) {
    case 1: {
        std::string title;
        std::cout << "Enter film title: ";
        std::getline(std::cin, title);

        // Приводим введённое название к нижнему регистру
        std::string titleLower = toLower(title);

        // Получаем все фильмы и фильтруем вручную с учётом регистра
        std::vector<Film*> found;
        for (auto& film : catalog.getAllFilms()) {
            if (toLower(film.getTitle()) == titleLower) {
                found.push_back(&film);
            }
        }
        displaySearchResults(found);
        break;
    }
    case 2: {
        std::cout << "\nAvailable genres:\n";
        for (size_t i = 0; i < GENRES.size(); ++i) {
            std::cout << i + 1 << ". " << GENRES[i] << "\n";
        }
        int genreChoice = readInt("Choose a genre (1-" + std::to_string(GENRES.size()) + "): ",
            1, (int)GENRES.size());
        displaySearchResults(catalog.findFilmsByGenre(GENRES[genreChoice - 1]));
        break;
    }
    case 3: {
        int year = readInt("Enter release year: ", 1900, 2026);
        displaySearchResults(catalog.findFilmsByYear(year));
        break;
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
            std::string sourcePath = selected->getMetadata();
            std::string userDir = "users/" + currentUser->getLogin();
            std::string fileName = std::filesystem::path(sourcePath).filename().string();
            std::string destPath = userDir + "/" + fileName;

            try {
                std::filesystem::copy(sourcePath, destPath,
                    std::filesystem::copy_options::overwrite_existing);
                std::cout << "Film downloaded to your folder: " << destPath << "\n";
                if (!selected->isAccessible()) {
                    std::cout << " (subscription access)\n";
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cout << "Error: Could not copy film file. " << e.what() << "\n";
            }
        }
        else {
            std::cout << "Film is unavailable. Subscription required.\n";
        }
    }
    else {
        Film* film = found[0];
        if (film->isAccessible() || currentUser->hasSubscription()) {
            std::string sourcePath = film->getMetadata();
            std::string userDir = "users/" + currentUser->getLogin();
            std::string fileName = std::filesystem::path(sourcePath).filename().string();
            std::string destPath = userDir + "/" + fileName;

            try {
                std::filesystem::copy(sourcePath, destPath,
                    std::filesystem::copy_options::overwrite_existing);
                std::cout << "Film downloaded to your folder: " << destPath << "\n";
                if (!film->isAccessible()) {
                    std::cout << " (subscription access)\n";
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cout << "Error: Could not copy film file. " << e.what() << "\n";
            }
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

    // Запрашиваем код активации
    std::string activationCode;
    std::cout << "Enter activation code (6 digits): ";
    std::getline(std::cin, activationCode);

    // Загружаем коды из файла
    auto codes = loadPurchaseCodes();
    std::string codeHash = hashString(activationCode);

    // Ищем совпадение по хэшу и типу подписки
    bool found = false;
    for (auto it = codes.begin(); it != codes.end(); ++it) {
        if (it->first == codeHash && it->second == choice) {
            // Код найден — удаляем его и активируем подписку
            codes.erase(it);
            savePurchaseCodes(codes);
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "Invalid activation code or it doesn't match the selected period.\n";
        waitForEnter();
        return;
    }

    currentUser->buySubscription(months);
    saveUsers();

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
    std::string title, metadata, isan;
    int year;

    std::cout << "Enter title: ";
    std::getline(std::cin, title);

    std::cout << "\nAvailable genres:\n";
    for (size_t i = 0; i < GENRES.size(); ++i) {
        std::cout << i + 1 << ". " << GENRES[i] << "\n";
    }
    int genreChoice = readInt("Choose a genre (1-" + std::to_string(GENRES.size()) + "): ",
        1, (int)GENRES.size());
    std::string genre = GENRES[genreChoice - 1];

    year = readInt("Enter release year: ", 1900, 2026);

    std::cout << "Enter ISAN (e.g., 0000-0000-0000-0000-X): ";
    std::getline(std::cin, isan);

    if (catalog.findFilmByIsan(isan) != nullptr) {
        std::cout << "Error: A film with this ISAN already exists!\n";
        waitForEnter();
        return;
    }

    // Ввод пути к файлу с проверкой на пустоту
    do {
        std::cout << "Enter file path (metadata): ";
        std::getline(std::cin, metadata);
        if (metadata.empty()) {
            std::cout << "File path cannot be empty. Please try again.\n";
        }
    } while (metadata.empty());

    admin.addFilm(catalog, title, genre, year, metadata, isan);
    saveFilms();
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
        saveFilms();
        std::cout << "Film removed.\n";
    }
    else {
        int idToRemove = found[0]->getId();
        admin.removeFilm(catalog, title, idToRemove);
        saveFilms();
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
    saveFilms();
    std::cout << "Access to \"" << filmTitle
        << "\" (ID: " << idToChange << ") has been changed.\n";
    waitForEnter();
}