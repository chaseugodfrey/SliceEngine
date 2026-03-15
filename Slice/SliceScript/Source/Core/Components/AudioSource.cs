using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{


    public class AudioSource : Component
    {
        public AudioSource() { }
        public AudioSource(GameObject entity)
        {
            gameObject = entity;
        }

        public enum VolumeRollOff
        {
            Logarithmic = 0,
            Linear = 1
        }

        /// <summary>
        /// Triggers the sound to play.
        /// Honors the component's MinInterval.
        /// </summary>
        public void Play()
        {
            FunctionCalls.Audio_Play(gameObject.mID);
        }

        /// <summary>
        /// Stops the currently playing sound on this component.
        /// </summary>
        public void Stop()
        {
            FunctionCalls.Audio_Stop(gameObject.mID);
        }

        /// <summary>
        /// Is the channel currently playing?
        /// </summary>
        public bool IsPlaying
        {
            get { return FunctionCalls.Audio_IsPlaying(gameObject.mID); }
        }

        /// <summary>
        /// Get or set the paused state.
        /// </summary>
        public bool IsPaused
        {
            get { return FunctionCalls.Audio_GetPaused(gameObject.mID); }
            set { FunctionCalls.Audio_SetPaused(gameObject.mID, value); }
        }

        /// <summary>
        /// Get or set the loop state.
        /// </summary>
        public bool IsLoop
        {
            get { return FunctionCalls.Audio_GetLoop(gameObject.mID); }
            set { FunctionCalls.Audio_SetLoop(gameObject.mID, value); }
        }

        /// <summary>
        /// Get or set the volume.
        /// </summary>
        public float Volume
        {
            get { return FunctionCalls.Audio_GetVolume(gameObject.mID); }
            set { FunctionCalls.Audio_SetVolume(gameObject.mID, value); }
        }

        /// <summary>
        /// Get or set the pitch.
        /// </summary>
        public float Pitch
        {
            get { return FunctionCalls.Audio_GetPitch(gameObject.mID); }
            set { FunctionCalls.Audio_SetPitch(gameObject.mID, value); }
        }

        /// <summary>
        /// Get or set spatial blend.
        /// </summary>
        public float SpatialBlend
        {
            get { return FunctionCalls.Audio_GetSpatialBlend(gameObject.mID); }
            set { FunctionCalls.Audio_SetSpatialBlend(gameObject.mID, value); }
        }

        /// <summary>
        /// Get or set the 2D stereo pan.
        /// </summary>
        public float StereoPan
        {
            get { return FunctionCalls.Audio_GetPan(gameObject.mID); }
            set { FunctionCalls.Audio_SetPan(gameObject.mID, value); }
        }

        /// <summary>
        /// Get or set the mute state.
        /// </summary>
        public bool IsMute
        {
            get { return FunctionCalls.Audio_GetMute(gameObject.mID); }
            set { FunctionCalls.Audio_SetMute(gameObject.mID, value); }
        }

        //public Audio audio
        //{


        //    get
        //    {
        //        //Console.WriteLine($"Entity ID:{Entity.mID}");
        //        string audioName = FunctionCalls.Audio_GetSoundName(gameObject.mID);
        //        return new Audio(audioName);
        //    }
        //    //set
        //    //{
        //    //    //Console.WriteLine($"Entity ID:{Entity.mID}");

        //    //    FunctionCalls.Audio_SetSoundName(Entity.mID, ref value);
        //    //}
        //}

    }


}

