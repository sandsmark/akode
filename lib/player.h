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

#include "akode_export.h"

namespace aKode {

class File;
class Sink;
class Decoder;
class Resampler;

//! An implementation of a multithreaded aKode-player

/*!
 * The Player interface provides a clean and simple interface to multithreaded playback.
 * Notice however that the Player interface itself is not reentrant and should thus only
 * be used by a single thread.
 */
class AKODE_EXPORT Player {
public:
    Player();
    ~Player();

    /*!
     * Opens a player that outputs to the sink \a sinkname (the "auto"-sink is recommended).
     * Returns false if the device cannot be opened.
     *
     * State: \a Closed -> \a Open
     */
    bool open(const char* sinkname);
    /*!
     * Closes the player and releases the sink
     * Valid in all states.
     *
     * State: \a Open -> \a Closed
     */
    void close();

    /*!
     * Load the file \a filename and prepare for playing.
     * Return false if the file cannot be loaded or decoded.
     *
     * State: \a Open -> \a Loaded
     */
    bool load(const char* filename);
    /*!
     * Unload the file and release any resources allocated while loaded
     *
     * State: \a Loaded -> \a Open
     */
    void unload();

    /*!
     * Start playing.
     *
     * State: \a Loaded -> \a Playing
     */
    void play();
    /*!
     * Stop playing and release any resources allocated while playing.
     *
     * State: \a Playing -> \a Loaded
     *        \a Paused -> \a Loaded
     */
    void stop();
    /*!
     * Waits for the file to finish playing (eof or error) and calls stop.
     *
     * State: \a Playing -> \a Loaded
     */
    void wait();
    /* Not implemented!
     * Detach the playing file, and bring the player back to Open state.
     * The detached file will stop and unload by itself when finished
     *
     * State: \a Playing -> \a Open
     */
    void detach();
    /*!
     * Pause the player.
     *
     * State: \a Playing -> \a Paused
     */
    void pause();
    /*!
     * Resume the player from paused.
     *
     * State: \a Paused -> \a Playing
     */
    void resume();

    /* Not implemented!
     * Prepare to crossfade current file, it can now be safely unloaded and a new file loaded.
     * The crossfade will last for up to \a ms milliseconds.
     */
    void crossfade(unsigned int ms) {};

    /*!
     * Set the software-volume to \a v. Use a number between 0.0 and 1.0.
     *
     * Valid in states \a Playing and \a Paused
     */
    void setVolume(float v);
    /*!
     * Returns the current value of the software-volume.
     *
     * Valid in states \a Playing and \a Paused
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
     *
     * Beware the callbacks come from a private thread, and methods from
     * the Player interface should not be called from the callback.
     */
    class Manager {
        Player* m_player;
        public:
            /*!
             * Called for all state changes
             */
            virtual void stateChangeEvent(Player::State) {};
            /*!
             * Called when a decoder halts because it has reached end of file.
             * The callee should effect a Player::stop()
             */
            virtual void eofEvent() {};
            /*!
             * Called when a decoder halts because of a fatal error.
             * The callee should effect a Player::stop()
             */
            virtual void errorEvent() {};
    };

    /*!
     * Sets an associated callback interface
     */
    void setManager(Manager *manager);

    /*!
     * Sets the decoder plugin to use. Default is auto-detect.
     */
    void setDecoderPlugin(const char* plugin);
    /*!
     * Sets the resampler plugin to use. Default is "fast".
     */
    void setResamplerPlugin(const char* plugin);

    struct private_data;
private:
    private_data *d;
    void setState(State state);
};

} // namespace

#endif
