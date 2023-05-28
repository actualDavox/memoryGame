#include <algorithm>
#include <clocale>
#include <fstream>
#include <iostream>
#include "sleep_ms.h"
#include <string>
#include <Windows.h>

#define MAX_PLAYERS 100
#define MAX_LEVEL 99
#define MAX_LIVES 3
#define BEEP_DURATION 500

struct UserCredentials
{
    std::string username;
    std::string password;
};

struct Player
{
    std::string fullname;
    std::string password;
    std::string username;
    int current_level;
};



std::string generate_cpu_sequence(const size_t);
std::string format_to_cpu_seq(const std::string&);
std::string similarity_idx_to_string(const float idx);
std::string pick_random_encouragement();
UserCredentials get_user_credentials();



bool validate_creds(UserCredentials, Player[], const size_t);
bool validate_sequence(const std::string&, const std::string&, const int);

float calculate_ratio_similarity(const std::string&, const std::string&);

int find_player_idx_by_username(const std::string& , Player[], const size_t);

void display_char_of_seq(const char);
void display_user_info(const size_t, Player[]);
void load_players_from_file(const std::string&, Player[], size_t&);
void play_game(Player[], size_t);
void play_losing_sound();
void play_winning_sound(const int);
void save_player_data_to_file(const std::string&, const Player[], size_t );






int main()
{

    std::setlocale(LC_ALL, "");

    size_t array_size{ MAX_PLAYERS };
    std::string file_name{ "input.txt" };
    bool is_logged{ false };
    Player player_array[MAX_PLAYERS];
    
    load_players_from_file(file_name, player_array, array_size);
    UserCredentials user_creds;

    do
    {

        user_creds = get_user_credentials();

        is_logged = validate_creds(user_creds, player_array, array_size);
        if (!is_logged)
        {
            std::cout << "Nom d'utilisateur ou mot de passe invalide.\nVeuillez réessayer.\n\n";
        }
    } while (!is_logged);

    const int user_idx{ find_player_idx_by_username(user_creds.username, player_array, array_size) };

    play_game(player_array, user_idx);
    save_player_data_to_file(file_name, player_array, array_size);
    std::cout << "Vous n'avez plus de vies, mais votre niveau a été sauvegardé. Veuillez redémarrer le jeu.";

}



std::string format_to_cpu_seq(const std::string& user_seq) {
    std::string formatted{};
    for (char c : user_seq) {
        if (std::isdigit(c)) {
            formatted += c;
        }
    }
    return formatted;
}

std::string generate_cpu_sequence(const size_t level) {
    std::string cpu_seq{};
    srand(time(NULL));

    for (int i{ 0 }; i < level; ++i) {
        cpu_seq += static_cast<char>((rand() % 10) + 48);
    }
    return cpu_seq;
}

std::string pick_random_encouragement() {
    std::string messages[]{ "Exactement!", "Parfait!", "C'est bien cela!", "Impressionant!", "Fantastique!", "Bravo!", "Bien joué", "Continue comme cela!" };
    return messages[rand() % 8] + "\n";
}

std::string similarity_idx_to_string(const float idx) {
    
    if (idx >= 0.8) {
        return "Tu y étais presque, mais mauvaise réponse!\n";
    }
    else if (idx > 0.6) {
        return "Bien tenté, mais incorrect!\n";
    }
    else if (idx > 0.4) {
        return "Porte une attention particulière aux chiffres affichés!\n";
    }
    else if (idx > 0.2) {
        return "Tiens bon. Pratiques-toi pour t'améliorer!\n";
    }
    else {
        return "Tu dois retranscrire les chiffres affichés à l'écran!\n";
    }
}



UserCredentials get_user_credentials()
{

    UserCredentials user_creds;
    std::cout << "Nom d'utilisateur: \n";
    std::cin >> user_creds.username;

    std::cout << "Mot de passe: \n";
    std::cin >> user_creds.password;
    return user_creds;
}



bool validate_sequence(const std::string& user_seq, const std::string& cpu_seq, const int level)
{
    return format_to_cpu_seq(user_seq).substr(0, level) == cpu_seq.substr(0, level);
}


bool validate_creds(UserCredentials user_creds, Player player_array[], const size_t player_array_size)
{
    int player_idx{ find_player_idx_by_username(user_creds.username, player_array, player_array_size) };

    if (player_idx != -1) {
        return user_creds.password == player_array[player_idx].password;
    }
    return false;

}


/*
Function returns a float between 0 and 1 representing how similar two strings are. If they're the same, 1 will be returned. 
If characters are all different, 0 will be returned.
*/
float calculate_ratio_similarity(const std::string& user_seq, const std::string& cpu_seq) {
    
    int nb_same_values{ 0 };
    for (int i{ 0 }; i < user_seq.size(); ++i) {
        if (user_seq[i] == cpu_seq[i]) {
            nb_same_values++;
        }
    }
    float val{ static_cast<float>(nb_same_values) / user_seq.size() };
    return static_cast<float>(nb_same_values) / user_seq.size();

}


int find_player_idx_by_username(const std::string& username, Player player_array[], const size_t player_array_size)
{
    for (int i{ 0 }; i < player_array_size; ++i)
    {
        if (player_array[i].username == username)
        {
            return i;
        }
    }
    return -1;
}

void display_char_of_seq(const char c) {
    std::cout << c << " ";
    sleep_ms(1000);
}

void display_user_info(const size_t user_index, Player player_array[]) {
    std::cout << "Nom: " << player_array[user_index].fullname << "\nNiveau: " << player_array[user_index].current_level << "\n\n";
};


void load_players_from_file(const std::string& input_file_name, Player player_array[], size_t& array_size)
{
    std::ifstream input_file{ input_file_name };
    std::string line{};
    size_t i{ 0 };

    constexpr size_t PASSWORD_LENGTH{ 6 };
    constexpr size_t USERNAME_LENGTH{ 6 };
    constexpr size_t FULL_NAME_MAX_LENGTH{ 32 };


    while (std::getline(input_file, line) && i < MAX_PLAYERS)
    {
        player_array[i].fullname = line.substr(0, line.find("  "));
        player_array[i].current_level = std::stoi(line.substr(FULL_NAME_MAX_LENGTH + USERNAME_LENGTH + PASSWORD_LENGTH));
        player_array[i].username = line.substr(FULL_NAME_MAX_LENGTH, USERNAME_LENGTH);
        player_array[i].password = line.substr((FULL_NAME_MAX_LENGTH + USERNAME_LENGTH), PASSWORD_LENGTH);
        ++i;
    }
    array_size = i;
}


void play_losing_sound() {
    constexpr int LOSS_FREQUENCY{ 300 };
    Beep(LOSS_FREQUENCY, BEEP_DURATION);
}


void display_text_pre_game(const size_t user_id, Player players[]) {
    constexpr int COUNTDOWN_TIME{ 3 };
    for (int i{ COUNTDOWN_TIME}; i >= 0; --i) {
        display_user_info(user_id, players);
        std::cout << "Mémorise la séquence affichée! Fais attention, tu n'as que trois vies!\n";
        std::cout << "La partie commence dans " << i << " secondes.";
        sleep_ms(1000);
        system("cls");
    }
}

void play_game(Player players[], size_t user_id) {

    int lives{ MAX_LIVES };
    int current_level{ players[user_id].current_level };
    std::string user_sequence{};
    std::string cpu_sequence{ generate_cpu_sequence(MAX_LEVEL) };
    std::cin.ignore();

    
    display_text_pre_game(user_id, players);


    while (lives > 0 && current_level < MAX_LEVEL) {
        std::cout << "SÉQUENCE: \n\n";
        for (int i{ 0 }; i < current_level; i++) {
            display_char_of_seq(cpu_sequence[i]);
        }
        system("cls");

        std::cout << "Entrez la séquence: ";
        std::getline(std::cin, user_sequence);
        bool is_valid_sequence(validate_sequence(user_sequence, cpu_sequence, current_level));

        if (!is_valid_sequence) {
            float index_of_similarity{ calculate_ratio_similarity(format_to_cpu_seq(user_sequence), cpu_sequence) };
            std::string message{ similarity_idx_to_string(index_of_similarity) };
            std::cout << message;
            play_losing_sound();
            lives--;
        }
        else {

            std::cout << pick_random_encouragement();
            play_winning_sound(current_level);
            current_level++;
        }
        sleep_ms(2000);
    }
    players[user_id].current_level = current_level;

    if (current_level > 99) {
        std::cout << "Félications, tu as terminé le jeu! Tu as une mémoire de dauphin!";
    }
}

void play_winning_sound(const int level) {
    constexpr int DURATION_MS{ 500 };
    constexpr int MAX_SOUND_LEVEL{ 15 };
    constexpr int MAX_FREQUENCY{ 950 }; 

    int frequency{ 500 + (level * 30) };
    frequency = (frequency > MAX_FREQUENCY) ? MAX_FREQUENCY : frequency;
    std::cout << frequency;
    Beep(frequency, DURATION_MS);
}
void save_player_data_to_file(const std::string& savegame_file_name, const Player players[], size_t array_size) {
    std::ofstream save_game{ savegame_file_name };
    for (int i{ 0 }; i < array_size; ++i) {
        save_game << players[i].fullname;
        for (int j{ 0 }; j < 32 - players[i].fullname.size(); j++) {
            save_game << " ";
        }
        save_game << players[i].username << players[i].password << players[i].current_level << "\n";
    }
}