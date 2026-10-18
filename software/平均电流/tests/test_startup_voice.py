"""验证满幅归一化后的语音与 Flash 一致，拒绝空音频和错误采样格式。"""
import importlib.util
from pathlib import Path
import re
import tempfile
import unittest
import wave

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("pack_voice", ROOT / "tools/audio/pack_startup_voice.py")
packer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packer)


def decode(header):
    count = int(re.search(r"BUZZER_VOICE_SAMPLE_COUNT (\d+)U", header)[1])
    words = [int(value, 16) for value in re.findall(r"0x([0-9A-F]{4})U", header)]
    return bytes(sample for word in words for sample in (word & 255, word >> 8))[:count]


class VoiceTests(unittest.TestCase):
    def test_flash_matches_peak_normalized_source(self):
        with wave.open(str(ROOT / "tools/audio/startup_voice.wav"), "rb") as wav:
            self.assertEqual((wav.getframerate(), wav.getnchannels(), wav.getsampwidth()), (8000, 1, 1))
            self.assertEqual(wav.getnframes(), 8402)
            data = wav.readframes(wav.getnframes())
        header = (ROOT / "Codes/includes/startup_voice.h").read_text()
        normalized = packer.normalize_pcm(data)
        self.assertEqual(decode(header), normalized)
        self.assertEqual(max(normalized), 255)
        self.assertGreater(max(data) - min(data), 16)
        with tempfile.TemporaryDirectory() as folder:
            regenerated = Path(folder) / "voice.h"
            packer.pack_voice(ROOT / "tools/audio/startup_voice.wav", regenerated)
            self.assertEqual(regenerated.read_text(), header)

    def test_odd_sample_count_and_invalid_audio(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "input.wav"
            header = Path(folder) / "voice.h"
            for data, rate, valid in [(bytes([10, 128, 245]), 8000, True),
                                      (b"", 8000, False), (bytes([128]) * 10, 8000, False),
                                      (bytes([10, 128, 245]), 16000, False)]:
                with wave.open(str(path), "wb") as wav:
                    wav.setparams((1, 1, rate, 0, "NONE", "not compressed"))
                    wav.writeframes(data)
                if valid:
                    packer.pack_voice(path, header)
                    self.assertEqual(decode(header.read_text()), packer.normalize_pcm(data))
                else:
                    with self.assertRaises(ValueError):
                        packer.pack_voice(path, header)

    def test_peak_normalization_preserves_silence_and_full_scale(self):
        self.assertEqual(packer.normalize_pcm(bytes([85, 128, 206])), bytes([58, 128, 255]))
        full_scale = bytes([0, 128, 255])
        self.assertEqual(packer.normalize_pcm(full_scale), full_scale)


if __name__ == "__main__":
    unittest.main()
