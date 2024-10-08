#include "chessEngine.h"

using namespace std;

Stockfish::~Stockfish() {
    if (stockfishProcess) {
        pclose(stockfishProcess);
    }
}

void Stockfish::init() {
    stockfishProcess = popen((filesystem::current_path() / "stockfish").string().c_str(), "r+");
    if (!stockfishProcess) {
        cerr << "Error starting Stockfish." << endl;
        return;
    }
    sendCommand("uci");
    // sendCommand("setoption name UCI_LimitStrength value true");
    // sendCommand("setoption name Skill Level value 10");
}

void Stockfish::sendCommand(const string& command) {
    fputs((command + "\n").c_str(), stockfishProcess);
    fflush(stockfishProcess);
}

string Stockfish::readResponse() {
    char buffer[256];
    string response;
    while (fgets(buffer, sizeof(buffer), stockfishProcess)) {
        response += buffer;
        if (strstr(buffer, "bestmove")) {
            break;  // Stop reading after we find the best move
        }
    }
    return response;
}

void Stockfish::setDifficulty(int difficulty) {
    if (difficulty < 0 || difficulty > 20) {
        difficulty = min(max(difficulty, 0), 20);
        cerr << "Difficulty must be between 0 and 20!" << endl;
        cerr << "Defaulting to " << to_string(difficulty) << endl;
    }
    this->difficulty = difficulty;
    sendCommand("setoption name Skill Level value " + to_string(difficulty));
}

string Stockfish::getMove(const string& boardPosition) {
    if (remoteProcessing) {
        try {
            return getMoveRemote(boardPosition);
        } catch (exception& e) {
            cerr << "Error processing move remotely: " << e.what() << endl;
            return getMoveLocal(boardPosition);
        }
    } else {
        return getMoveLocal(boardPosition);
    }
}

string Stockfish::getMoveLocal(const string& boardPosition) {
    // Set the position of the board
    sendCommand("position fen " + boardPosition);
    // Request the best move
    sendCommand("go depth " + to_string(depth));

    // Read and return the best move from Stockfish's response
    string response = readResponse();

    // Check if the response contains "bestmove"
    if (response.find("bestmove") == string::npos) {
        cerr << "Error: 'bestmove' not found in response: " << response << endl;
        return ""; // Return empty string if "bestmove" is not found
    }

    size_t bestMovePos = response.find("bestmove");
    if (bestMovePos == string::npos || bestMovePos + 9 >= response.size()) {
        cerr << "Error: 'bestmove' position is out of range in response: " << response << endl;
        return ""; // Return empty string if "bestmove" position is out of range
    }

    string bestMoveStr = response.substr(bestMovePos + 9);
    size_t spacePos = bestMoveStr.find(' '); // Find the first space
    if (spacePos != string::npos) {
        bestMoveStr = bestMoveStr.substr(0, spacePos);
        if (bestMoveStr == "(none)") {
            return "";
        } else {
            return bestMoveStr;
        }
    } else {
        return bestMoveStr;
    }
}

string Stockfish::getMoveRemote(const string& boardPosition) {
    // Create a temporary CURL object for escaping
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize CURL" << endl;
        return "";
    }

    // Create the URL with parameters
    string url = "https://stockfish.online/api/s/v2.php?fen=" + 
                      string(curl_easy_escape(curl, boardPosition.c_str(), boardPosition.length())) +
                      "&depth=" + to_string(depth);

    // Clean up CURL object
    curl_easy_cleanup(curl);

    // Perform the API request
    string response = sendGetRequest(url);

    // Parse the response to extract the best move
    return parseMove(response);
}

string Stockfish::sendGetRequest(const string& url) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    // Initialize CURL
    curl = curl_easy_init();
    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the write callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
    
    return readBuffer;
}

size_t Stockfish::WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

string Stockfish::parseMove(const string& response) {
    // Parse the response JSON and extract the best move
    nlohmann::json jsonResponse;

    // Try parsing the JSON response
    try {
        jsonResponse = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error& e) {
        cerr << "JSON parse error: " << e.what() << endl;
        return ""; // Return empty string on parse error
    }

    // Check for the "bestmove" key in the response
    if (jsonResponse.contains("bestmove")) {
        // Get the best move string
        string bestMoveStr = jsonResponse["bestmove"].get<string>().substr(9);

        size_t spacePos = bestMoveStr.find(' '); // Find the first space
        if (spacePos != string::npos) {
            return bestMoveStr.substr(0, spacePos); // Return the substring up to the first space
        } else {
            // If no space is found, return the whole string as the best move
            return bestMoveStr;
        }
    } else {
        cerr << "Error: 'bestmove' not found in response: " << response << endl;
        return ""; // Return empty string if "bestmove" is not found
    }
}