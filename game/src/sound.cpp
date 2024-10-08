#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>

using namespace std;

// Declare sound buffers globally
sf::SoundBuffer startBuffer;
sf::SoundBuffer moveBuffer;
sf::SoundBuffer illegalBuffer;
sf::SoundBuffer castleBuffer;
sf::SoundBuffer captureBuffer;
sf::SoundBuffer checkBuffer;
sf::SoundBuffer checkmateBuffer;

sf::Sound startSound;
sf::Sound moveSound;
sf::Sound illegalSound;
sf::Sound castleSound;
sf::Sound captureSound;
sf::Sound checkSound;
sf::Sound checkmateSound;
sf::Music backgroundMusic;

void initSound() {
    // Load the sound buffers
    if (!startBuffer.loadFromFile("assets/audio/start.mp3")) {
        cerr << "Error loading start sound!" << endl;
    }
    startSound.setBuffer(startBuffer);

    if (!moveBuffer.loadFromFile("assets/audio/move.mp3")) {
        cerr << "Error loading move sound!" << endl;
    }
    moveSound.setBuffer(moveBuffer);

    if (!illegalBuffer.loadFromFile("assets/audio/illegal.mp3")) {
        cerr << "Error loading illegal sound!" << endl;
    }
    illegalSound.setBuffer(illegalBuffer);

    if (!castleBuffer.loadFromFile("assets/audio/castle.mp3")) {
        cerr << "Error loading castle sound!" << endl;
    }
    castleSound.setBuffer(castleBuffer);

    if (!captureBuffer.loadFromFile("assets/audio/capture.mp3")) {
        cerr << "Error loading capture sound!" << endl;
    }
    captureSound.setBuffer(captureBuffer);

    if (!checkBuffer.loadFromFile("assets/audio/check.mp3")) {
        cerr << "Error loading check sound!" << endl;
    }
    checkSound.setBuffer(checkBuffer);

    if (!checkmateBuffer.loadFromFile("assets/audio/checkmate.mp3")) {
        cerr << "Error loading checkmate sound!" << endl;
    }
    checkmateSound.setBuffer(checkmateBuffer);

    // Load and play the background music
    if (!backgroundMusic.openFromFile("assets/audio/background.mp3")) {
        cerr << "Error loading background music!" << endl;
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(25);
    backgroundMusic.play();
}