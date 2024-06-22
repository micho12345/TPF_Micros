/***************************************************************************//**
  @file     SDManager.h
  @brief    Mike Towers Baby
  @authors  Micho
 ******************************************************************************/

#ifndef _MUSIC_
#define _MUSIC_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "diskio.h"
#include "fsl_clock.h"
#include "board.h"
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
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

uint8_t SD_Inity (uint8_t songs[ALL_SONGS][NAME_SIZE]);

void PlaySong(char *name, uint8_t volumen);

uint32_t Mp3ReadId3V2Tag(FIL* pInFile, char* pszArtist, uint32_t unArtistSize, char* pszTitle, uint32_t unTitleSize);

void PauseSong(void);

void ResumeSong(void);

void getSongName(uint8_t SongName[1][NAME_SIZE]);

/*******************************************************************************
 ******************************************************************************/

#endif // _MUSIC_
