/*  aKode: Player class

    Copyright (C) 2004-2005 Allan Sandfeld Jensen <kde@carewolf.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _AKODE_PLAYER_H
#define _AKODE_PLAYER_H
#include <kdemacros.h>
#include <string>
using std::string;

namespace aKode {

class File;
class Sink;
class Decoder;
class Resampler;

//! An implementation of a multithreaded aKode-player

/*!
 */
class KDE_EXPORT Player {
public:
    Player();
    ~Player();

    /*!
     * Opens a player that outputs to the sink \a sinkname (the "auto"-sink is recommended).
     */
    bool open(string sinkname);
    /*!
     * Closes the player and releases the sink
     * Valid in all states.
     */
    void close();

    /*!
     * Load the file \a filename and prepare for playing.
     */
    bool load(string filename);
    /*!
     * Unload the file and release any resources allocated while loaded
     */
    void unload();

    /*!
     * Start playing.
     */
    void play();
    /*!
     * Pause the player.
     */
    void pause();
    /*!
     * Resume the player from paused.
     */
    void resume();
    /*!
     * Stop playing and release any resources allocated while playing.
     */
    void stop();

    /*!
     * Prepare to crossfade current file, it can now be safely unloaded and a new file loaded.
     * The crossfade will last for up to \a ms milliseconds.
     */
    void crossfade(unsigned int ms) {};

    /*!
     * Set the software-volume to \a v. Use a number between 0.0 and 1.0.
     */
    void setVolume(float v);
    /*!
     * Returns the current value of the software-volume.
     */
    float volume() const;

    File* file() const;
    Sink* sink() const;
    /*!
     * Returns the current decoder interface.
     * Used for seek, position and length.
     */
    Decoder* decoder() const;
    /*!
     * Returns the current resampler interface.
     * Used for adjusting playback speed.
     */
    Resampler* resampler() const;

    enum State { Closed  = 0,
                 Open    = 2,
                 Loaded  = 4,
                 Playing = 8,
                 Paused  = 12 };

    /*!
     * Returns the current state of the Player
     */
    State state() const;

    /*!
     * An interface for Player callbacks
     * Beware the callbacks are all from a local thread
     */
    class Manager {
        Player* m_player;
        public:
            /*!
             * Called for all state changes
             */
            virtual void stateChangeEvent(Player::State) {};
            /*!
             * Called when a decoder reaches end of file
             */
            virtual void eofEvent() {};
            /*!
             * Called when a decoder encounters a fatal error
             */
            virtual void errorEvent() {};
    };

    /*!
     * Sets an associated callback interface
     */
    void setManager(Manager *manager);

    struct private_data;
private:
    private_data *m_data;
    void setState(State state);
};

} // namespace

#endif
