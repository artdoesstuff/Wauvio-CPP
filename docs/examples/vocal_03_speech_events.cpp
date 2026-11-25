// Speech and non-lexical sounds: say() for spoken lines, event() / <tags> for laughs, hums and more.
#include "wauvio/vocal/v3.hpp"
using namespace wauvio::vocal::v3;

int main() {
    Singer s;
    s.natural();

    s.say("こんにちは", 1.2, "statement");
    s.rest_sec(0.2);
    s.say("げんき", 0.9, "question");               // rising intonation
    s.rest_sec(0.3);
    s.say("はし", 0.7, "statement", 69, 0.75, /*accent nucleus*/ 1);   // Tokyo accent: HIGH-low (箸, chopsticks)
    s.rest_sec(0.5);

    s.event("laugh", 72, 1.0, /*intensity*/ 0.7);
    s.rest_sec(0.3);
    s.note_sec(72, 1.0, "<hum>");                   // the same thing written as a lyric tag

    s.save("vocal_03_speech_events.wav");
}
