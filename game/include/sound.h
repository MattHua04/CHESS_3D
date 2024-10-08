#ifndef SOUND_H
#define SOUND_H

#include "globals.h"

void initSound();

extern sf::Sound startSound;
extern sf::Sound moveSound;
extern sf::Sound illegalSound;
extern sf::Sound castleSound;
extern sf::Sound captureSound;
extern sf::Sound checkSound;
extern sf::Sound checkmateSound;
extern sf::Music backgroundMusic;

#endif