#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "memorymap.h"

// This variable is only used during simulation, to test the arbitration
// between CPU and Ethernet while writing to memory.
uint8_t dummy_counter = 0;

// Start of receive buffer
uint8_t *pBuf;

// Length of current frame
uint16_t frmLen;

const uint8_t arpHeader[10]   = {0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01};
const uint8_t myMacAddress[6] = {0x70, 0x4D, 0x7B, 0x11, 0x22, 0x33};  // AsustekC
const uint8_t myIpAddress[4]  = {192, 168, 1, 77};

void processFrame(uint8_t *rdPtr)
{
   printf("Got frame\n");

   // Set read pointer to past the length field
   rdPtr += 2;

   if (memcmp(rdPtr+12, arpHeader, 10))
      return;

   printf("Got ARP.\n");

   if (memcmp(rdPtr+38, myIpAddress, 4))
      return;

   printf("Bingo!\n");

   // Build new MAC header
   memcpy(rdPtr, rdPtr+6, 6);
   memcpy(rdPtr+6, myMacAddress, 6);

   // Build new ARP header
   rdPtr[21] = 2; // ARP Reply
   memcpy(rdPtr+32, rdPtr+22, 10); // Copy original senders MAC and IP address to the target.
   memcpy(rdPtr+22, myMacAddress, 6);
   memcpy(rdPtr+28, myIpAddress, 4);

   // Send reply
   MEMIO_CONFIG->ethTxPtr  = (uint16_t) rdPtr - 2;
   MEMIO_CONFIG->ethTxCtrl = 1;

   // Wait until frame has been consume by TxDMA.
   while (MEMIO_CONFIG->ethTxCtrl)
   {}
} // end of processFrame

void main(void)
{
   // Allocate receive buffer. This will never be free'd.
   // Using malloc (rather than a globally allocated array) avoids a call to
   // memset generated by the compiler, thereby reducing simulation time.
   // The size must be at least an entire Ethernet frame including 2-byte header.
   pBuf = (uint8_t *) malloc(1516);

   // Verify that Rx DMA is disabled from start.
   assert(MEMIO_CONFIG->ethRxdmaEnable == 0);

   // Configure Ethernet DMA. May only be done while Rx DMA is disabled.
   MEMIO_CONFIG->ethRxdmaPtr = (uint16_t) pBuf;
   
   // Wait for data to be received, and print to the screen
   while (1)
   {
      // Indicate to Rx DMA that we are ready to receive a frame.
      MEMIO_CONFIG->ethRxdmaEnable = 1;

      // Wait until an ethernet frame has been received
      while (MEMIO_CONFIG->ethRxdmaEnable == 1)
      {
         dummy_counter += 1;   // This generates a write to the main memory.
      }

      // Calculate length of frame
      frmLen = *((uint16_t *)pBuf);  // Read as little-endian.

      processFrame(pBuf);
   }

} // end of main
