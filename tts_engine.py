#!/usr/bin/env python3
"""
KittenTTS Backend for C++ Application
Generates high-quality speech using KittenTTS
Runs on CPU, no GPU required

Usage:
    python tts_engine.py synthesize "Hello world" female_0 output.wav
    python tts_engine.py list_voices
    python tts_engine.py clone_voice input_audio.wav my_voice_name
"""

import sys
import json
import os
from pathlib import Path
import traceback

# Try to import KittenTTS, with helpful error message if not installed
try:
    from kitten_tts import KittenTTS
except ImportError:
    print(json.dumps({
        "status": "error",
        "error": "KittenTTS not installed. Install with: pip install kitten-tts"
    }))
    sys.exit(1)


def generate_speech(text, voice="female_0", output_file="output.wav", 
                    language="en", speed=1.0):
    """
    Generate speech using KittenTTS
    
    Args:
        text: Text to synthesize
        voice: Voice name (female_0, male_0, etc. or custom voice ID)
        output_file: Output WAV file path
        language: Language code (en, fr, de, es, zh, ja, etc.)
        speed: Speech speed (0.5-2.0)
    
    Returns:
        JSON with status and metadata
    """
    try:
        # Initialize TTS engine (cached after first run)
        print(f"[DEBUG] Initializing KittenTTS...", file=sys.stderr)
        tts = KittenTTS()
        
        print(f"[DEBUG] Synthesizing text with voice '{voice}'...", file=sys.stderr)
        
        # Generate audio
        # KittenTTS returns numpy array of audio samples
        audio = tts.synthesize(
            text,
            voice=voice,
            language=language,
            speed=speed
        )
        
        # Ensure output directory exists
        output_path = Path(output_file)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        print(f"[DEBUG] Saving audio to {output_file}...", file=sys.stderr)
        
        # Save as WAV file (24 kHz, mono, 16-bit)
        tts.save_wav(audio, output_file, sample_rate=24000)
        
        # Calculate duration
        audio_length = len(audio)  # Number of samples
        sample_rate = 24000
        duration = audio_length / sample_rate
        file_size = output_path.stat().st_size
        
        print(f"[DEBUG] Audio saved. Duration: {duration:.2f}s, Size: {file_size} bytes", 
              file=sys.stderr)
        
        return {
            "status": "success",
            "file": output_file,
            "duration": duration,
            "sample_rate": 24000,
            "channels": 1,
            "file_size": file_size,
            "voice": voice,
            "language": language,
            "speed": speed
        }
        
    except Exception as e:
        print(f"[ERROR] {traceback.format_exc()}", file=sys.stderr)
        return {
            "status": "error",
            "error": str(e),
            "type": type(e).__name__
        }


def list_voices():
    """
    List all available voices
    
    Returns:
        JSON with list of voice names and properties
    """
    try:
        print(f"[DEBUG] Loading KittenTTS to list voices...", file=sys.stderr)
        tts = KittenTTS()
        
        # Get available voices
        # KittenTTS has built-in voices: female_0-4, male_0-3, plus 100+ other voices
        built_in_voices = [
            # Female voices (higher pitch)
            {"id": "female_0", "name": "Female 1 (Default)", "gender": "female"},
            {"id": "female_1", "name": "Female 2", "gender": "female"},
            {"id": "female_2", "name": "Female 3", "gender": "female"},
            {"id": "female_3", "name": "Female 4", "gender": "female"},
            {"id": "female_4", "name": "Female 5", "gender": "female"},
            
            # Male voices (lower pitch)
            {"id": "male_0", "name": "Male 1", "gender": "male"},
            {"id": "male_1", "name": "Male 2", "gender": "male"},
            {"id": "male_2", "name": "Male 3", "gender": "male"},
            {"id": "male_3", "name": "Male 4", "gender": "male"},
        ]
        
        # KittenTTS supports 20 languages
        languages = [
            "en", "fr", "de", "es", "it", "pt", "ru", "zh", "ja", "ko",
            "ar", "hi", "tr", "nl", "sv", "da", "fi", "sw", "el", "he"
        ]
        
        return {
            "status": "success",
            "voices": built_in_voices,
            "total_voices": len(built_in_voices),
            "supported_languages": languages,
            "voice_cloning": "supported",
            "expression_control": "supported",
            "model": "kitten-tts-2",
            "sample_rate": 24000,
            "note": "Use female_0 or male_0 for standard voices. Clone custom voices from audio."
        }
        
    except Exception as e:
        return {
            "status": "error",
            "error": str(e)
        }


def clone_voice(audio_file, voice_name="custom_voice"):
    """
    Clone a voice from audio file
    
    Args:
        audio_file: Path to reference audio (5-30 seconds)
        voice_name: Name for the cloned voice
    
    Returns:
        JSON with cloned voice ID
    """
    try:
        if not os.path.exists(audio_file):
            return {
                "status": "error",
                "error": f"Audio file not found: {audio_file}"
            }
        
        print(f"[DEBUG] Cloning voice from {audio_file}...", file=sys.stderr)
        tts = KittenTTS()
        
        # Clone voice from reference audio
        cloned_voice_id = tts.clone_voice(
            reference_audio_path=audio_file,
            voice_name=voice_name
        )
        
        return {
            "status": "success",
            "voice_id": cloned_voice_id,
            "voice_name": voice_name,
            "reference_audio": audio_file,
            "message": f"Voice '{voice_name}' cloned successfully"
        }
        
    except Exception as e:
        return {
            "status": "error",
            "error": str(e)
        }


def get_info():
    """Get engine information"""
    return {
        "status": "success",
        "engine": "KittenTTS",
        "version": "2.0",
        "capabilities": {
            "speech_synthesis": True,
            "voice_cloning": True,
            "expression_control": True,
            "emotion_tags": True,
            "emphasis": True,
            "multiple_languages": True,
            "gpu_optional": True,
            "cpu_realtime": True
        },
        "features": [
            "115+ built-in voices",
            "Voice cloning (5-30 seconds audio)",
            "20 language support",
            "Expression control with emotion tags",
            "Real-time synthesis on CPU",
            "24 kHz high-quality audio"
        ],
        "sample_rate": 24000,
        "channels": 1,
        "format": "WAV (16-bit PCM)"
    }


def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("Usage: python tts_engine.py <command> [args]", file=sys.stderr)
        print("Commands:", file=sys.stderr)
        print("  synthesize <text> <voice> <output_file> [language] [speed]", file=sys.stderr)
        print("  list_voices", file=sys.stderr)
        print("  clone_voice <audio_file> <voice_name>", file=sys.stderr)
        print("  info", file=sys.stderr)
        print("", file=sys.stderr)
        print("Example:", file=sys.stderr)
        print('  python tts_engine.py synthesize "Hello world" female_0 output.wav', 
              file=sys.stderr)
        sys.exit(1)
    
    command = sys.argv[1]
    result = None
    
    try:
        if command == "synthesize" and len(sys.argv) >= 5:
            text = sys.argv[2]
            voice = sys.argv[3]
            output_file = sys.argv[4]
            language = sys.argv[5] if len(sys.argv) > 5 else "en"
            speed = float(sys.argv[6]) if len(sys.argv) > 6 else 1.0
            
            result = generate_speech(text, voice, output_file, language, speed)
            
        elif command == "list_voices":
            result = list_voices()
            
        elif command == "clone_voice" and len(sys.argv) >= 4:
            audio_file = sys.argv[2]
            voice_name = sys.argv[3]
            result = clone_voice(audio_file, voice_name)
            
        elif command == "info":
            result = get_info()
            
        else:
            result = {
                "status": "error",
                "error": f"Unknown command: {command}",
                "available_commands": ["synthesize", "list_voices", "clone_voice", "info"]
            }
        
        # Output JSON result
        print(json.dumps(result))
        
        # Exit with appropriate code
        sys.exit(0 if result.get("status") == "success" else 1)
        
    except Exception as e:
        print(json.dumps({
            "status": "error",
            "error": str(e),
            "traceback": traceback.format_exc()
        }))
        sys.exit(1)


if __name__ == "__main__":
    main()
