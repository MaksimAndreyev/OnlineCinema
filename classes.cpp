#include "classes.h"
#include <algorithm>
#include <cmath>

// ==================== User ====================

User::User() : authorized(false), subscriptionEnd(0) {}

void User::registerUser(const std::string& login, const std::string& password) {
    this->login = login;
    this->password = password;
    authorized = true;
    subscriptionEnd = 0;
}

void User::loginUser(const std::string& login, const std::string& password) {
    if (this->login == login && this->password == password) {
        authorized = true;
    }
}

void User::logout() {
    authorized = false;
}

void User::watchFilm(const std::string& filmTitle) {
    // Реализуется через взаимодействие с Catalog в main.cpp
}

void User::buySubscription(int months) {
    int days = months * 30;
    if (subscriptionEnd == 0 || subscriptionEnd < std::time(nullptr)) {
        subscriptionEnd = std::time(nullptr) + days * 86400;
    }
    else {
        subscriptionEnd += days * 86400;
    }
}

void User::rateFilm(Catalog& catalog, std::vector<Review>& reviews,
    const std::string& filmTitle, int idToRate, int score, const std::string& text) {
    reviews.emplace_back(score, text, idToRate, login);

    // Пересчитываем средний рейтинг фильма
    double totalScore = 0;
    int count = 0;
    for (const auto& review : reviews) {
        if (review.getFilmId() == idToRate) {
            totalScore += review.getScore();
            ++count;
        }
    }

    Film* film = catalog.getFilmById(idToRate);
    if (film && count > 0) {
        film->updateRating(totalScore / count);
    }
}

std::string User::getLogin() const {
    return login;
}

bool User::isAuthorized() const {
    return authorized;
}

bool User::hasSubscription() const {
    return authorized && subscriptionEnd > std::time(nullptr);
}


// ==================== Administrator ====================

Administrator::Administrator() {}

void Administrator::addFilm(Catalog& catalog, const std::string& title, const std::string& genre, int year, const std::string& metadata) {
    int id = catalog.getAllFilms().size() + 1;
    catalog.addFilm(Film(id, title, genre, year, metadata));
}

void Administrator::removeFilm(Catalog& catalog, const std::string& title, int idToRemove) {
    catalog.removeFilmById(idToRemove);
}

void Administrator::changeFilmAccess(Catalog& catalog, const std::string& title, int idToChange, bool accessible) {
    Film* film = catalog.getFilmById(idToChange);
    if (film) {
        film->setAccessible(accessible);
    }
}

// ==================== Film ====================

Film::Film() : id(0), rating(0.0), accessible(true), year(0) {}

Film::Film(int id, const std::string& title, const std::string& genre, int year,
    const std::string& metadata)
    : id(id), title(title), genre(genre), year(year), rating(0.0),
    accessible(true), metadata(metadata) {}

int Film::getId() const { return id; }
std::string Film::getTitle() const { return title; }
std::string Film::getGenre() const { return genre; }
int Film::getYear() const { return year; }
double Film::getRating() const { return rating; }
bool Film::isAccessible() const { return accessible; }
std::string Film::getMetadata() const { return metadata; }

void Film::setAccessible(bool accessible) {
    this->accessible = accessible;
}

void Film::updateRating(double newRating) {
    rating = std::round(newRating * 10) / 10;
}

// ==================== Review ====================

Review::Review() : score(0), filmId(0) {}

Review::Review(int score, const std::string& text, int filmId, const std::string& authorLogin)
    : score(score), text(text), filmId(filmId), authorLogin(authorLogin) {}

int Review::getScore() const { return score; }
std::string Review::getText() const { return text; }
int Review::getFilmId() const { return filmId; }
std::string Review::getAuthorLogin() const { return authorLogin; }

// ==================== Catalog ====================

Catalog::Catalog() {}

void Catalog::addFilm(const Film& film) {
    films.push_back(film);
}

void Catalog::removeFilm(const std::string& title) {
    films.erase(
        std::remove_if(films.begin(), films.end(),
            [&title](const Film& f) { return f.getTitle() == title; }),
        films.end()
    );
}

void Catalog::removeFilmById(int id) {
    films.erase(
        std::remove_if(films.begin(), films.end(),
            [id](const Film& f) { return f.getId() == id; }),
        films.end()
    );
}

std::vector<Film*> Catalog::findFilms(const std::string& title) {
    std::vector<Film*> result;
    for (auto& film : films) {
        if (film.getTitle() == title) {
            result.push_back(&film);
        }
    }
    return result;
}

Film* Catalog::getFilmById(int id) {
    for (auto& film : films) {
        if (film.getId() == id) {
            return &film;
        }
    }
    return nullptr;
}

std::vector<Film>& Catalog::getAllFilms() {
    return films;
}