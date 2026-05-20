#pragma once
#include <string>
#include <ctime>
#include <vector>

class Catalog;
class Review;

class User {
private:
    bool authorized;
    std::time_t subscriptionEnd;
    std::string login;
    std::string passwordHash;

public:
    User();

    void registerUser(const std::string& login, const std::string& passwordHash);
    void loginUser(const std::string& login, const std::string& passwordHash);
    void logout();
    void watchFilm(const std::string& filmTitle);
    void buySubscription(int months);
    void rateFilm(Catalog& catalog, std::vector<Review>& reviews,
        const std::string& filmTitle, int idToRate, int score, const std::string& text);
    void setSubscriptionEnd(std::time_t end);
    std::string getLogin() const;
    std::time_t getSubscriptionEnd() const;
    std::string getPasswordHash() const;
    bool isAuthorized() const;
    bool hasSubscription() const;
};

class Administrator {
public:
    Administrator();

    void addFilm(Catalog& catalog, const std::string& title, const std::string& genre,
        int year, const std::string& metadata, const std::string& isan);
    void removeFilm(Catalog& catalog, const std::string& title, int idToRemove);
    void changeFilmAccess(Catalog& catalog, const std::string& title, int idToChange, bool accessible);
};

class Film {
private:
    int id;
    std::string title;
    std::string genre;
    int year;
    double rating;
    bool accessible;
    std::string metadata;
    std::string isan;  // International Standard Audiovisual Number

public:
    Film();
    Film(int id, const std::string& title, const std::string& genre, int year,
        const std::string& metadata, const std::string& isan);

    int getId() const;
    std::string getTitle() const;
    std::string getGenre() const;
    int getYear() const;
    double getRating() const;
    bool isAccessible() const;
    std::string getMetadata() const;
    std::string getIsan() const;

    void setAccessible(bool accessible);
    void updateRating(double newRating);
};

class Review {
private:
    int score;
    std::string text;
    int filmId;
    std::string authorLogin;

public:
    Review();
    Review(int score, const std::string& text, int filmId, const std::string& authorLogin);

    int getScore() const;
    std::string getText() const;
    int getFilmId() const;
    std::string getAuthorLogin() const;
};

class Catalog {
private:
    std::vector<Film> films;

public:
    Catalog();

    void addFilm(const Film& film);
    void removeFilm(const std::string& title);
    void removeFilmById(int id);
    std::vector<Film*> findFilms(const std::string& title);
    std::vector<Film*> findFilmsByGenre(const std::string& genre);
    std::vector<Film*> findFilmsByYear(int year);
    Film* findFilmByIsan(const std::string& isan);
    Film* getFilmById(int id);
    std::vector<Film>& getAllFilms();
};