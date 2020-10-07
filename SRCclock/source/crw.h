#ifndef CRW_H
#define CRW_H

#include <string>
#include <fstream>
#include <pulse/simple.h>
#include <pulse/error.h>
//#include <pulse/pulseaudio.h>

class Crw {
int INstate;            // Stato dello stream in input
int OUTstate;           // Stato dello stream in output

std::fstream fd;          // Descrizione del file per l'input e l'output su file
pa_simple*  sound_stream;         // Descrizione dello stream di PulseAudio

int sound_error;            // Variabile che identifica l'ultimo errore nello stream del suono

public:

/**
 * @brief Leggi i dati dallo stream di input che è stato precedentemente aperto
 *
 * @param Buffer dei dati da leggere
 * @param samples Numero di campioni da leggere. Nessun controllo viene eseguito sulle dimensioni dell'array di buffer
 * @param size Dimensione in bytes di ciascun elemento del buffer. Di default è settato a 1
 * @return <int> Numero di campioni letti
 **/

virtual int readBuffer(void* buffer, int samples, int size = 1);      // Leggi un buffer dallo stream di input aperto

/**
 * @brief Scrive dei dati allo stream di input che è stato precedentemente aperto
 *
 * @param buffer Buffer di dati da scrivere
 * @param samples Numero di campioni da scrivere. Nessun controllo viene eseguito sulle dimensioni dell'array di buffer
 * @param size Dimensione in bytes di ciascun elemento del buffer. Di default è settato a 1
 * @return <int> Dimensione in bytes di ciascun elemento del buffer. Di default è settato a 1
 **/

virtual int writeBuffer(const void* buffer, int samples, int size = 1);   // Scrivi un buffer a uno stream aperto di output

/**
 * @brief Apre un semplice stream di PulseAudio in playback
 *
 * @param fc Frequenza di campionamento
 * @param canali 1 = mono; 2 = stereo
 * @param SampFormat Formato di campionamento
 * @param device Nome del dispositivo di PulseAudio
 * @param appName Nome dell'applicazione
 * @return <bool> Ritorna True se tutto è OK
 **/

virtual bool open_soundStream_output(int fc, int channels, const char* device = 0, pa_sample_format SampFormat = PA_SAMPLE_U8, const char* appName = 0);

/**
 * @brief Apre un semplice stream di PulseAudio in registrazione
 *
 * @param fc Frequenza di campionamento
 * @param canali 1 = mono; 2 = stereo
 * @param SampFormat Formato di campionamento
 * @param device Nome del dispositivo di PulseAudio
 * @param appName Nome dell'applicazione
 * @return <bool> Ritorna True se tutto è OK
 **/

virtual bool open_soundStream_input(int fc, int channels, const char* device = 0, pa_sample_format SampFormat = PA_SAMPLE_U8, const char* appName = 0);


virtual bool open_file_output(const char* fileNAme);      // Apre lo stream di output su file
virtual bool open_file_input(const char* fileNAme);       // Apre lo stream di input su file

virtual void close_input_stream();            // Chiude lo stream di input
virtual void close_output_stream();           // Chiude lo stream di output
virtual void close_all();             // Chiude tutti gli streams



std::string get_sound_error() const {
        return pa_strerror(sound_error);
}                                                                             // Ritorna la stringa del precedente errore di PulseAudio


int get_INstate() const {
        return INstate;
}                                                  // Ritorna il valore della variable INstate per lo stato dell'input
int get_OUTstate() const {
        return OUTstate;
}                                                 // Ritorna il valore della variable OUTstate per lo stato dell'output

Crw();
virtual ~Crw() {
        close_all();
}
};

#endif // CRW_H
