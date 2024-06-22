/***************************************************************************//**
  @file     SDManager.c
  @brief    Mike Towers Baby
  @author   Micho
  @cite		https://github.com/vinodstanur/frdmk64f_mp3_player/blob/master/source/main.c
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <hardware.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <MK64F12.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDManager.h>			//del archivo
#include "UI/MCAL/gpio.h"
#include "board.h"

#include "../helix/pub/mp3dec.h"		//helix

// Fresscale Libs
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "diskio.h"
#include "ffconf.h"
#include "fsl_clock.h"
#include "UI/Pdrivers/headers/PIT.h"
#include "UI/Pdrivers/headers/DAC.h"
#include "UI/Pdrivers/headers/DMA.h"
#include "ff.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define ALL_SONGS 50
#define NAME_SIZE 16
#define BUFFER_SIZE (1024*NAME_SIZE)
#define STRING_SIZE 64
#define TIMER_ID 1
#define DAC_OFF_H 0x8U
#define DAC_OFF_L 0x00U
#define VOLUMEN_FACTOR 5
#define SAMPLE_BUFFER_SIZE 2304

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
HMP3Decoder  Decoder;		//Decoder
MP3FrameInfo Data_Frame;	//Data Frame

char  *buffer_ptr				;		//PING PONG BUFFER
uint8_t ping_pong_buffer[BUFFER_SIZE];			//puntero al buffer
static char artista[STRING_SIZE];			//artista
static char titulo[STRING_SIZE];			//nombre de la cancion
int16_t sample_buffer[SAMPLE_BUFFER_SIZE];	//sample buffer
int16_t *samples = sample_buffer;			//samples
float_t FloatSamples[SAMPLE_BUFFER_SIZE];	//samples
int16_t audio_buff[2304*2];		//buffer partido> un frame decodificando y otro leyendo con DMA (en paralelo)

/*******************************************************************************
 *         FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint32_t Mp3ReadId3V2Text(FIL* pInFile, uint32_t unDataLen, char* pszBuffer, uint32_t unBufferSize);

static void RunDAC(int sample_rate, int output_samples);

static void ProvideAudioBuffer(int16_t *samples, int cnt, uint8_t volumen);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/**************************************************************************************
 * Function:    SD_Init
 *
 * Description: Especie de Init de la SD
 *
 * Inputs:      Arreglo de canciones
 *
 * Outputs:     numero de canciones
 **************************************************************************************/
uint8_t SD_Inity (uint8_t songs[ALL_SONGS][NAME_SIZE]){
    uint8_t total_songs = 0;		//Numero total de canciones
    uint8_t index_songs = 0;		//Indice para ir moviendome en el dir

    /* File system object structure (FATFS) ----> ff.h */
    static FATFS SongsSystem;
    /* File function return code (FRESULT) -----> ff.h*/
    FRESULT error;
    /* Directory object structure (DIR)  -----> ff.h*/
    DIR directory;

	/* sd disk to physical drive 2  ---> SDDISK  */
    const TCHAR driverBuffer[3U] = {SDDISK + '0', ':', '/'};

    /* Mount/Unmount a logical drive */		//----> Me fijo si tiene error en volumen
    if (f_mount(&SongsSystem, driverBuffer, 0U)) {
		//printf("Falla en el volumen.\r\n");
		return -1;
	}

/* This option configures support of relative path.
/
/   0: Disable relative path and remove related functions.
/   1: Enable relative path. f_chdir() and f_chdrive() are available.
/   2: f_getcwd() function is available in addition to 1.
*/

    /* Change current directory */		//----> Me fijo si tiene error en cambiar de dir
	#if (_FS_RPATH >= 2U)
	  error = f_chdrive((char const *)&driverBuffer[0U]);
	  if (error) {
		//printf("Falla en cambiar el directorio.\r\n");
		return -1;
	  }
	#endif

    /* Open a directory */		//----> Me fijo si tiene error de abrir directorio
    if (f_opendir(&directory, "/")) {
		//printf("Falla en abrir el directorio.\r\n");
		return -1;
	}

    /* File information structure (FILINFO) */
    FILINFO files;		//ESTRUCTURA DE LA INFORMACION DEL ARCHIVO
	//FSIZE_t	fsize;			/* File size */
	//WORD	fdate;			/* Modified date */
	//WORD	ftime;			/* Modified time */
	//BYTE	fattrib;		/* File attribute */
	//TCHAR	altname[13];			/* Altenative file name */
	//TCHAR	fname[_MAX_LFN + 1];	/* Primary file name */
	//TCHAR	fname[13];		/* File name */
    /* File function return code (FRESULT) */


	FRESULT result;		//ERRORES. 0 es Succeded

    while(1) {
        /* Read a directory item */
		result =  f_readdir(&directory, &files);            //leemos directorio
		if(result != FR_OK || strlen(files.fname) == 0) {   //Si hay algun error o no hay para leer, entonces FUERA
		  break;							// termino
          //FR_OK ---> /* (0) Succeeded */
		}

		if(strstr(files.fname, ".MP3")) {                   //sigo leyendo los .mp3 y cuento lo como cancion
			strcpy( (char *) songs[index_songs], (char *) files.fname );      //Agarro files.fname y los copio en el arreglo mp3_files
			index_songs++;
			total_songs++;                                  //avanzo index y sumo cantidad de canciones
		}
	}
	return total_songs;         //termina y me devuelve la cantidad de canciones
}



//FALTA: EL VUMETRO
/**************************************************************************************
 * Function:    PlaySong
 *
 * Description: Pone play a la cancion
 *
 * Inputs:      Nombre de la cancion y ...
 *
 * Outputs:     -
 **************************************************************************************/
void PlaySong(char *name, uint8_t volumen) {

    /* File object structure (FIL) */
	static FIL file;        	//OBJETO FILE
    /* File function return code (FRESULT) */
	static FRESULT fresult;		//ERRORS

	static int bytes_faltantes;
	static int sync, error;
	static int outOfData;
	static uint8_t dac_on = 0;
	static bool open_file = false;
	static char *old_name = NULL;

    if(*old_name != *name){     			//Si es la primera vez que se ejecuta
		old_name = name;
		bytes_faltantes = 0;
		outOfData = 0;

		if(strlen(name) == 0) {             //Si la longitud es 0 entonces no hay canciones
			while(1);
		}

		if (open_file) {        //Si estaba abierto el archivo, simplemente lo cierro y cambio el estado de la variable
			f_close(&file);
			open_file = false;
		}

		if(f_open(&file,name, FA_READ) == FR_OK) {      //Abri el archivo, y si lo pudiste abrir bien, entonces poneme open_file en true
			open_file = true;
		}
		else {                                          //Si no pude abrir el archivo --->ERROR
			while(1);
		}

		Decoder = MP3InitDecoder();     //Hago el init del mp3 y guardo memoria dinamica para el
        //allocate memory for platform-specific data clear all the user-accessible fields


		//Aca rescato Metadata de la cancion, ya que los frames estan en el medio de la info
		//Rescato el nombre de la cancion y el artista
		Mp3ReadId3V2Tag(&file, artista, sizeof(artista), titulo, sizeof(titulo));
	}


	//Arranco con el PING PONG BUFFER: separo el buffer en dos, una mitad se decodea y la otra se manda al dac
    static unsigned int bytes_leidos, bytes_a_leer;
	//Si los bytes que faltan son menos de la mitad entonces entrame
	if( bytes_faltantes < BUFFER_SIZE/2 ) {              //Si me quedan menos que la mitad del buffer
		memcpy( (char *) ping_pong_buffer, (char *) buffer_ptr, bytes_faltantes );  //destino:ping_pong_buffer   source:buffer_ptr   #bytes:bytes_faltantes
		buffer_ptr = (char *) ping_pong_buffer;							//Igualo punteros  -->   pongo el puntero al principio del buffer
		bytes_a_leer = BUFFER_SIZE - bytes_faltantes;			//actualizo los bytes a leer

		// Prendo LED para mostrar al usuario que esta leyendo los archivos
		//GPIO_WritePinOutput(GPIOB, BOARD_LED_RED_GPIO_PIN , 0);
		fresult = f_read(&file, ping_pong_buffer + bytes_faltantes, bytes_a_leer, &bytes_leidos);
		//GPIO_WritePinOutput(GPIOB, BOARD_LED_RED_GPIO_PIN , 1);

		bytes_faltantes = BUFFER_SIZE;				//actualizo bytes_left con la cantidad entera del buffer

		if(fresult || bytes_leidos < bytes_a_leer) {	//Si hay error o los bytes leidos son menores a los bytes a leer entonces termino la cancion
			f_close(&file);								//Por lo tanto, cierro el archivo
			return;
		}
	}

    //locate the next byte-alinged sync word in the raw mp3 stream
    //buffer to search for sync word max number of bytes to search in buffer
    sync = MP3FindSyncWord((unsigned char*)buffer_ptr, bytes_faltantes);		//te busco el proximo que va a estar alineado
	//Busco la parte que son frames de cancion, entonces me la sincroniza al primer frame
	//Me devuelve lo que tengo que correr el buffer para sincronizarlo con el primer frame

	if(sync == -1 ) {
		bytes_faltantes = 0;     //-1 if sync not found after searching nBytes
		return;					//Si me devuelve -1 entonces no hubo sincronizacion ---> error
	}

	bytes_faltantes -= sync;   		//corro los bytes_faltantes por la sincronizacion --> tengo menos bytes para comer
	buffer_ptr += sync;		//corro el puntero del buffer a leer por la sincronizacion



    //decode one frame of MP3 data
	//Decodifico cada frame que le mando
    error = MP3Decode(Decoder, (unsigned char**)&buffer_ptr, (int*)&bytes_faltantes, samples, 0);
    //error code, defined in mp3dec.h (0 means no error, < 0 means error)

	if (error) {		//Hubo error, depende del error hago diferentes cosas
		switch (error) {
			case ERR_MP3_INDATA_UNDERFLOW:
				outOfData = 1;
			break;
			case ERR_MP3_MAINDATA_UNDERFLOW:
			break;
			case ERR_MP3_NULL_POINTER:
				bytes_faltantes -=1;
				buffer_ptr+=1;
			case ERR_MP3_FREE_BITRATE_SYNC:
			default:
				outOfData = 1;
			break;
		}
	}
    else {			//No hubo error, entonces que arranque la fiesta
        //get info about last MP3 frame decoded (number of sampled decoded, sample rate, bitrate, etc.)
        //El driver dice que lo tengo que llamar luego de MP3Decode, es decir, post decodear (INFO DEL FRAME)
		MP3GetLastFrameInfo(Decoder, &Data_Frame);
		if(!dac_on) {       //PRENDO EL DAC
			dac_on = 1;
			RunDAC(Data_Frame.samprate, Data_Frame.outputSamps);
			DAC_Enable(DAC_0, true);
		}
		//Si tiene un canal (si es mono), duplica los datos para mantener velocidad de reproduccion
		if (Data_Frame.nChans == 1) {
			for(int i = Data_Frame.outputSamps;i >= 0;i--) {
				samples[2 * i]=samples[i];		//lo pongo del otro lado
				samples[2 * i + 1]=samples[i];
			}
			Data_Frame.outputSamps *= 2;
		}
	}

	//Procesamiento de los samples post decodificacion
    if (!outOfData) {			//TO DO: JUNTARME CON CUTY A HACER EL VUMETRO

		ProvideAudioBuffer(samples, Data_Frame.outputSamps, volumen);


 		//VUMETRO

	}


}




/*
* Taken from
* http://www.mikrocontroller.net/topic/252319
*/
//INDU
uint32_t Mp3ReadId3V2Tag(FIL* pInFile, char* pszArtist, uint32_t unArtistSize, char* pszTitle, uint32_t unTitleSize){
  pszArtist[0] = 0;
  pszTitle[0] = 0;

  BYTE id3hd[10];
  UINT unRead = 0;
  if((f_read(pInFile, id3hd, 10, &unRead) != FR_OK) || (unRead != 10)){
    return 1;
  }
  else{
    uint32_t unSkip = 0;
    if((unRead == 10) &&
       (id3hd[0] == 'I') &&
         (id3hd[1] == 'D') &&
           (id3hd[2] == '3'))
    {
      unSkip += 10;
      unSkip = ((id3hd[6] & 0x7f) << 21) | ((id3hd[7] & 0x7f) << 14) | ((id3hd[8] & 0x7f) << 7) | (id3hd[9] & 0x7f);

      // try to get some information from the tag
      // skip the extended header, if present
      uint8_t unVersion = id3hd[3];
      if(id3hd[5] & 0x40){
        BYTE exhd[4];
        f_read(pInFile, exhd, 4, &unRead);
        size_t unExHdrSkip = ((exhd[0] & 0x7f) << 21) | ((exhd[1] & 0x7f) << 14) | ((exhd[2] & 0x7f) << 7) | (exhd[3] & 0x7f);
        unExHdrSkip -= 4;
        if(f_lseek(pInFile, f_tell(pInFile) + unExHdrSkip) != FR_OK){
          return 1;
        }
      }
      uint32_t nFramesToRead = 2;
      while(nFramesToRead > 0){
        char frhd[10];
        if((f_read(pInFile, frhd, 10, &unRead) != FR_OK) || (unRead != 10)){
          return 1;
        }
        if((frhd[0] == 0) || (strncmp(frhd, "3DI", 3) == 0)){
          break;
        }
        char szFrameId[5] = {0, 0, 0, 0, 0};
        memcpy(szFrameId, frhd, 4);
        uint32_t unFrameSize = 0;
        uint32_t i = 0;
        for(; i < 4; i++){
          if(unVersion == 3){
            // ID3v2.3
            unFrameSize <<= 8;
            unFrameSize += frhd[i + 4];
          }
          if(unVersion == 4){
            // ID3v2.4
            unFrameSize <<= 7;
            unFrameSize += frhd[i + 4] & 0x7F;
          }
        }

        if(strcmp(szFrameId, "TPE1") == 0){
          // artist
          if(Mp3ReadId3V2Text(pInFile, unFrameSize, pszArtist, unArtistSize) != 0){
            break;
          }
          nFramesToRead--;
        }
        else if(strcmp(szFrameId, "TIT2") == 0){
          // title
          if(Mp3ReadId3V2Text(pInFile, unFrameSize, pszTitle, unTitleSize) != 0){
            break;
          }
          nFramesToRead--;
        }
        else{
          if(f_lseek(pInFile, f_tell(pInFile) + unFrameSize) != FR_OK){
            return 1;
          }
        }
      }
    }
    if(f_lseek(pInFile, unSkip) != FR_OK){
      return 1;
    }
  }

  return 0;
}



/**************************************************************************************
 * Function:    PauseSong
 *
 * Description: Pone pausa a la cancion
 *
 * Inputs:      -
 *
 * Outputs:     -
 **************************************************************************************/
void PauseSong(void){
	PIT_stopTime(TIMER_ID);				//Paro el timer pasandole el ID
	DAC0->DAT[0].DATH = DAC_OFF_H;		//Para darme cuenta que del DAC solo salga esto y ande bien
	DAC0->DAT[0].DATL = DAC_OFF_L;
}

/**************************************************************************************
 * Function:    ResumeSong
 *
 * Description: Pone play a la cancion
 *
 * Inputs:      -
 *
 * Outputs:     -
 **************************************************************************************/
void ResumeSong(void) {
	PIT_startTime(TIMER_ID);		//Le doy play al timer pasandole el ID
}

/**************************************************************************************
 * Function:    getSongName
 *
 * Description: Es un getter del nombre de la cancion
 *
 * Inputs:      Songname
 *
 * Outputs:     -
 **************************************************************************************/
void getSongName(uint8_t SongName[1][NAME_SIZE]){
	strcpy( (char*) SongName[0], titulo);    		//Guardo los nombres
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*
* Taken from
* http://www.mikrocontroller.net/topic/252319
*/
//INDU
static uint32_t Mp3ReadId3V2Text(FIL* pInFile, uint32_t unDataLen, char* pszBuffer, uint32_t unBufferSize){
  UINT unRead = 0;
  BYTE byEncoding = 0;
  if((f_read(pInFile, &byEncoding, 1, &unRead) == FR_OK) && (unRead == 1)){
    unDataLen--;
    if(unDataLen <= (unBufferSize - 1)){
      if((f_read(pInFile, pszBuffer, unDataLen, &unRead) == FR_OK) ||
         (unRead == unDataLen))
      {
        if(byEncoding == 0){
          // ISO-8859-1 multibyte
          // just add a terminating zero
          pszBuffer[unDataLen] = 0;
        }
        else if(byEncoding == 1){
          // UTF16LE unicode
          uint32_t r = 0;
          uint32_t w = 0;
          if((unDataLen > 2) && (pszBuffer[0] == 0xFF) && (pszBuffer[1] == 0xFE)){
            // ignore BOM, assume LE
            r = 2;
          }
          for(; r < unDataLen; r += 2, w += 1){
            // should be acceptable for 7 bit ascii
            pszBuffer[w] = pszBuffer[r];
          }
          pszBuffer[w] = 0;
        }
      }
      else{
        return 1;
      }
    }
    else{
      // we won't read a partial text
      if(f_lseek(pInFile, f_tell(pInFile) + unDataLen) != FR_OK){
        return 1;
      }
    }
  }
  else{
    return 1;
  }
  return 0;
}




/**************************************************************************************
 * Function:    RunDAC
 *
 * Description: Hace que se reproduzca la cancion habilitando el DAC
 *
 * Inputs:      sample_rate y output_samples
 *
 * Outputs:     -
 **************************************************************************************/
static void RunDAC(int sample_rate, int output_samples) {

	initDAC(DAC_0);			//Inicializo el DAC

	// DMA Config
	DMA_config_t DMAconfig = {.channel = DMA_1, .source_buffer = sample_buffer,
								 .destination_buffer = &(DAC0->DAT), .source_offset = sizeof(uint32_t), .transfer_size = sizeof(uint16_t),
								 .source_full_size = output_samples*sizeof(uint32_t), .source_unit_size = sizeof(uint32_t), .request_source = DMAALWAYS63};

	DMA_Config(DMAconfig);

	PIT_init();				//Inicializo el Timer y hago su configuracion
	PIT_configTimer(TIMER_ID,((CLOCK_GetFreq(kCLOCK_BusClk) / (sample_rate))));		//El clock lo saco de fsl_clock
	PIT_startTime(TIMER_ID);		//Le doy play al timer pasandole el ID
}


/**************************************************************************************
 * Function:    ProvideAudioBuffer
 *
 * Description: Le provee los samples de audio al audio buffer
 *
 * Inputs:      Las samples, el contador y el volumen
 *
 * Outputs:     -
 **************************************************************************************/

static void ProvideAudioBuffer(int16_t *samples, int total_samples, uint8_t volumen) {
	static uint8_t state = 0;

	//Ecualizador
	/*
	if ( GetOnOffEq()) {							//Si el ecualizador esta en ON
		float32_t input_filter_buffer[SAMPLE_BUFFER_SIZE];	//Uso un buffer para la entrada del filtro
		float32_t output_filter_buffer[SAMPLE_BUFFER_SIZE];	//Uso otro para la salida del filtro
		for (int i = 0; i < total_samples; i++) {
			input_filter_buffer[i] = (float32_t)samples[i];	//Pongo las samples en el buffer
		}
		offEqualizer(input_filter_buffer, output_filter_buffer, total_samples);	//Hago el proceso del filtro

		for (int i = 0; i < total_samples; i++) {		//Ahora pongo lo resultante del filtro en las samples
			samples[i] = (int16_t) output_filter_buffer[i];
		}
	}			//Aca ya esta sintetizada
*/
	//Ajusto el Volumen: del Indu
	int32_t promedio = 0;
	uint8_t volume = volumen * VOLUMEN_FACTOR;							//Como es de 20 niveles, se multiplica por 5
	for(int i = 0; i < total_samples; i++) {							//para que el nivel 20 sea el 100%
		if(i%2 == 0) {													//Si es par
			promedio =   (samples[i] + samples[i+1])/2;					//promedio de ambas samples
			samples[i] = (int16_t)promedio * (int32_t)volume / 100;		//Modifico volumen de la sample (regla de 3)
		}
	}

	//Hago el PING PONG buffer: primer mitad
	if(state == 0) {
		while( DMA_GetRemainingMajorLoopCount(DMA_1) > total_samples/2 ) {}
		//Lleva la cuenta de los bytes que faltan transferir, cuando llega a la mitad, arranca con el otro estado

		for(int i = 0; i < total_samples; i++) {			//INDU
			audio_buff[i] = *samples / NAME_SIZE;
			audio_buff[i] += (4096/2);
			samples++;
		}
		state = 1;
		return;
	}

	//Hago el PING PONG buffer: segunda mitad
	if(state == 1) {
		while( DMA_GetRemainingMajorLoopCount(DMA_1) < total_samples/2 ) { }
		//Lo mismo que arriba, cuando se pasa de la mitad, cambia al otro estado. Asi se hace el ping pong

		for(int i = 0; i < total_samples; i++) {				//INDU
			audio_buff[i + total_samples] = *samples / NAME_SIZE;
			audio_buff[i + total_samples] += (4096/2);
			samples++;
		}
		state = 0;
	}
}
