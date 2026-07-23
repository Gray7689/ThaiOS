// ThaiAI - ThaiOS Integrated AI Assistant
// ===========================================
// Assistente AI locale con NLP, controllo sistema,
// riconoscimento vocale e automazione.

#ifndef _THAI_AI_H
#define _THAI_AI_H

#include <thaios.h>

#define THAI_AI_MAX_MODELS 8
#define THAI_AI_MAX_AUTOMATIONS 128
#define THAI_AI_MAX_CONTEXT 4096

typedef enum ai_model_type {
    AI_MODEL_NONE,
    AI_MODEL_LLM,           // Large Language Model (llama.cpp)
    AI_MODEL_WHISPER,       // Speech-to-text
    AI_MODEL_IMAGE,         // Image recognition
    AI_MODEL_CODE           // Code generation
} ai_model_type_t;

typedef struct ai_model {
    char name[64];
    ai_model_type_t type;
    char model_path[512];
    bool loaded;
    usize context_size;
    usize memory_usage;
    int (*infer)(struct ai_model *model, const char *input, char *output, usize max_out);
    void *priv;
} ai_model_t;

typedef struct ai_automation {
    char name[64];
    char trigger[256];       // NLP trigger phrase
    char action[1024];       // Action to execute
    bool enabled;
    u64 trigger_count;
} ai_automation_t;

typedef struct thai_ai_state {
    ai_model_t models[THAI_AI_MAX_MODELS];
    int model_count;
    ai_automation_t automations[THAI_AI_MAX_AUTOMATIONS];
    int automation_count;
    char system_prompt[4096];
    bool voice_active;
    struct {
        float confidence;
        char text[4096];
    } last_transcription;
} thai_ai_state_t;

typedef enum ai_intent {
    INTENT_UNKNOWN,
    INTENT_OPEN_APP,
    INTENT_SEARCH_FILE,
    INTENT_SYSTEM_INFO,
    INTENT_RUN_COMMAND,
    INTENT_TRANSLATE,
    INTENT_SUMMARIZE,
    INTENT_GENERATE_CODE,
    INTENT_CREATE_AUTOMATION,
    INTENT_SET_REMINDER,
    INTENT_CONTROL_HARDWARE
} ai_intent_t;

// Core API
void thai_ai_init(void);
int thai_ai_load_model(const char *name, ai_model_type_t type, const char *path);
int thai_ai_unload_model(const char *name);
int thai_ai_query(const char *input, char *output, usize max_out);

// Intent recognition
ai_intent_t thai_ai_classify_intent(const char *input);
char **thai_ai_extract_entities(const char *input, int *count);

// Actions
int thai_ai_open_app(const char *app_name);
int thai_ai_search_files(const char *query, char *results, usize max_size);
int thai_ai_get_system_info(char *info, usize max_size);
int thai_ai_run_command(const char *command);
int thai_ai_translate(const char *text, const char *lang, char *out, usize max_out);
int thai_ai_summarize(const char *text, char *out, usize max_out);
int thai_ai_generate_code(const char *prompt, const char *lang, char *out, usize max_out);

// Automations
int thai_ai_create_automation(const char *name, const char *trigger, const char *action);
int thai_ai_remove_automation(const char *name);
int thai_ai_trigger_automation(const char *input);

// Voice
int thai_ai_voice_init(const char *model_path);
int thai_ai_voice_listen(void);         // Record from mic
int thai_ai_voice_transcribe(void);     // Transcribe to text
int thai_ai_voice_speak(const char *text); // TTS output

#endif
