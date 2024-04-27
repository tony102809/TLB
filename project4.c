//
//  project4.c
//  
//
//
//
#include "project4.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


#define TLB_SIZE 8
#define PAGE_TABLE_SIZE 16
#define CACHE_SIZE 64

/**
 
 This program  reads information about an existing TLB,  Virtual Address Page table, and Physical Memory Direct Memory Caching.
 Based on storing this information, the program will prompt the user a virtual address (in hex) and output the byte at that location.
 The program may also print out "Can not be determined", if the data is not available.
 
 @author: Tony Liu
 */

/**
 Structs that define the TLB, Page, the Virtual Address Page table, and Caching
 */
// Structure to represent a TLB entry
typedef struct tlb{
    char setIndex[4];
    char tag[4];
    char ppn[4];
} tlb;

// Structure to represent a page table entry
typedef struct page {
    char vpn[4];    // Virtual Page Number
    char ppn[4];    // Physical Page Number
} page;

// Structure to represent a cache entry
typedef struct cache {
    char cacheIndex[4];
    char tag[4];    // Tag bits
    char byte_offset0[4];
    char byte_offset1[4];
    char byte_offset2[4];
    char byte_offset3[4];
} cache;


// arrays to hold TLB, page table, and cache entries
//ints representing the sizes of those arrays, respectively
tlb tlb_array[100];
int numTlb;
page page_array[100];
int numPage;
cache cache_array[100];
int numCache;
char hex[5];

/**
 This function reads all the entries and separates them into their correct places. It parses all given data and populates the tlb, page, and cache arrays.
 @param: FILE representing the input file
 */
void read_entries(FILE *input_file) {
    int tlb_count = 0;
    int page_count = 0;
    char line[300];

    while (fgets(line, 300, input_file)) {
        if (line[0] == 'T') {   // Read TLB entry
            char setindex[4], tag[4], ppn[4];
            sscanf(line, "TLB,%[^,],%[^,],%[^,\n]", setindex, tag, ppn);
            tlb new_tlb;
            strcpy(new_tlb.setIndex, setindex);
            strcpy(new_tlb.tag, tag);
            strcpy(new_tlb.ppn, ppn);
            tlb_array[tlb_count] = new_tlb;
            tlb_count = tlb_count + 1;
            numTlb = numTlb + 1;
        } else if (line[0] == 'P') {   // Read page table entry
            char vpn[4], ppn[4];
            sscanf(line, "Page,%[^,],%[^,\n]", vpn, ppn);
            page new_page;
            strcpy(new_page.vpn, vpn);
            strcpy(new_page.ppn, ppn);
            page_array[page_count] = new_page;
            page_count = page_count + 1 ;
            numPage = numPage + 1;
        } else if (line[0] == 'C') {   // Read cache entry
            char cacheIndex[4], tag[4], byte_offset0[4], byte_offset1[4], byte_offset2[4], byte_offset3[4];
            sscanf(line, "Cache,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,\n]", cacheIndex, tag, byte_offset0, byte_offset1, byte_offset2, byte_offset3);
            cache new_cache;
            strcpy(new_cache.cacheIndex, cacheIndex);
            strcpy(new_cache.tag, tag);
            strcpy(new_cache.byte_offset0, byte_offset0);
            strcpy(new_cache.byte_offset1, byte_offset1);
            strcpy(new_cache.byte_offset2, byte_offset2);
            strcpy(new_cache.byte_offset3, byte_offset3);
            cache_array[numCache] = new_cache;
            numCache = numCache + 1;
        }
        else{ //For When it is Done
            break;
        }
    }
}
/**
 This function converts numbers from binary to hex
 @param: char* representing the character array to be converted
 @return: char* representing that character array in hex
 */
char* binaryToHex(char* binary) {
    
    int d = strtol(binary, NULL, 2);
    char* hexStr = (char*) malloc(3 * sizeof(char));
    sprintf(hexStr, "%02X", d);
    hexStr[2] = '\0';
    
    return hexStr;
}
/**
 This function converts numbers from hex to binary
 @param: char* representing the character array to be converted
 @return: char* representing that character array in binary
 */
char* hexToBinary(char* hex) {
    // Allocate space for the binary string
    char* binary = malloc((4 * strlen(hex) + 1) * sizeof(char));

    // Convert each hexadecimal digit to binary
    for (int i = 0; i < strlen(hex); i++) {
        char digit = hex[i];
        int decimal = 0;

        // Convert hex digit to decimal
        if (digit >= '0' && digit <= '9') {
            decimal = digit - '0';
        } else if (digit >= 'A' && digit <= 'F') {
            decimal = 10 + digit - 'A';
        } else if (digit >= 'a' && digit <= 'f') {
            decimal = 10 + digit - 'a';
        } else {
            // Invalid hex digit
            free(binary);
            return NULL;
        }

        // Convert decimal to binary
        for (int j = 0; j < 4; j++) {
            binary[4 * i + (3 - j)] = (decimal % 2 == 1) ? '1' : '0';
            decimal /= 2;
        }
    }

    // Add null terminator to binary string
    binary[4 * strlen(hex)] = '\0';

    return binary;
}
/**
 This method converts character strings to ints. This is used when character strings are just a single digit in the program.
 @param: char* representing the character array to be converted
 @return: int representing that character array
 */
int strToTnt(char* str) {
    int result = 0; // Initialize result to 0
    int i = 0; // Initialize index i to 0
    
    while (str[i] != '\0') { // Loop through each character in the string
        result = result * 10 + (str[i] - '0'); // Convert the current character to an integer and add it to the result
        i++; // Increment the index
    }
    
    return result;
}
/**
 This method actually does the mapping. The user enters their desired virtual address in hex, it is convereted to binary in the main function, and then It is fed into this function.
 @param: char* representing what the user entered as their virtual address
 */
void map_virtual_to_physical_address(char* binary) {
    
    
    //Divide up binary into tag, index, and offset
    char tag[5];    // tag
    char index[3];  // index
    char offset[7]; // offset
    int i;
    
    // extract tag, index, and offset from binary string
    for (i = 0; i < 4; i++) {
        tag[i] = binary[i];
    }
    tag[4] = '\0';
    for (i = 4; i < 6; i++) {
        index[i-4] = binary[i];
    }
    index[2] = '\0';
    for (i = 6; i < 12; i++) {
        offset[i-6] = binary[i];
    }
    offset[6] = '\0';
    
    
    char* ntag;    // new tag in hex
    char* nindex;  // new index in hex
    
    //Convert to hex
    ntag = binaryToHex(tag);
    nindex = binaryToHex(index);
    
    //Chop off leading 0 for index
    if (nindex[0] == '0') { // Check if first character is a zero
            nindex++; // Move the pointer to skip the zero
    }

    
    int found = -1;
    int a;
    for(a = 0; a < numTlb; a++){
        if(strcmp(tlb_array[a].tag, ntag) == 0 && strcmp(tlb_array[a].setIndex, nindex) == 0){
            found = 0;
            break;
        }
    }
            
    
    //SPECIAL CASE: Not Found
    //First 6 digits are VPN, match it to the VPN in Page Table. Take matching PPN...
    //In Cache table, match PPN to tag in cache table.. and then take cache index as is
    if(found == -1){
                
                char* vpn = (char*) malloc(7 * sizeof(char)); // Allocate memory for vpn
                
                strncpy(vpn, binary, 6); // Copy first 6 bits of binary into vpn
                vpn[6] = '\0'; // Add null terminator to the end of vpn
        
                char* hexVpn = binaryToHex(vpn);
        
                int i;
                int crash = -1;
                for(i = 0; i<numPage; i++){
                    if(strcmp(page_array[i].vpn, hexVpn)==0){
                        crash = 0;
                        break;
                    }
                }
                if(crash == -1){
                    printf("Can not be determined");
                    printf("\n");
                }
                
                char* tag = tlb_array[i].ppn;

                char* sci = (char*) malloc(5 * sizeof(char));
                char* sbo = (char*) malloc(3 * sizeof(char));
                
                // Extract 7th to 10th bits and put it into ci
                strncpy(sci, binary+6, 4);
                sci[4] = '\0'; // Add null terminator to the end of ci
                   
                // Extract 11th and 12th bits and put it into bo
                strncpy(sbo, binary+10, 2);
                sbo[2] = '\0'; // Add null terminator to the end of bo
                
                char* sfci = binaryToHex(sci);
                char* sfbo = binaryToHex(sbo);
                
                
                //Chop off leading 0 for fci and fbo
                if (sfci[0] == '0') { // Check if first character is a zero
                        sfci++; // Move the pointer to skip the zero
                }
                if (sfbo[0] == '0') { // Check if first character is a zero
                        sfbo++; // Move the pointer to skip the zero
                }
                
                int sffbo = strToTnt(sfbo);
                
                //sffbo contains the byte offset and sfci contains the cache index and tag contains the tag
                
                int o = -1;
                int l;
                for(l = 0; l<numCache; l++){
                    if(strcmp(cache_array[l].tag, tag)==0 && strcmp(cache_array[l].cacheIndex, sfci)==0){
                        if(sffbo == 0){
                            printf("%s", cache_array[l].byte_offset0);
                            o = 0;
                            printf("\n");
                        }
                        else if(sffbo == 1){
                            printf("%s", cache_array[l].byte_offset1);
                            o = 0;
                            printf("\n");
                        }
                        else if(sffbo == 2){
                            printf("%s", cache_array[l].byte_offset2);
                            o = 0;
                            printf("\n");
                        }
                        else{
                            printf("%s", cache_array[l].byte_offset3);
                            o = 0;
                            printf("\n");
                        }
                    }
                }
        if(o == -1 && crash == 0){
            printf("Can not be determined");
            printf("\n");
        }
    }
    
    else{
        //Now we can write the basic part of our physical address. We take the value of
        //the PPN we just got and then put the original offset after it. Our value is now : pppppp oooooo,
        //where the p's are digits of the PPN and o's are the digits of the offset.
        
        char* temp = tlb_array[a].ppn;
        char * matchPpn = hexToBinary(temp);
       
        
        //Match PPN to tag, which is in Hex, in Cache
        //Match digits 7, 8, 9, 10 of original virtual address in binary to cache index
        //Match last digits, 11 and 12 of original virtual address in binary to byteoffset
        
        //ci = cache index
        //bo = block offset
        char* ci = (char*) malloc(5 * sizeof(char));
        char* bo = (char*) malloc(3 * sizeof(char));
        
        // Extract 7th to 10th bits and put it into ci
        strncpy(ci, binary+6, 4);
        ci[4] = '\0'; // Add null terminator to the end of ci
           
        // Extract 11th and 12th bits and put it into bo
        strncpy(bo, binary+10, 2);
        bo[2] = '\0'; // Add null terminator to the end of bo
        
        char* fci = binaryToHex(ci);
        char* fbo = binaryToHex(bo);
        
        //Chop off leading 0 for fci and fbo
        if (fci[0] == '0') { // Check if first character is a zero
                fci++; // Move the pointer to skip the zero
        }
        if (fbo[0] == '0') { // Check if first character is a zero
                fbo++; // Move the pointer to skip the zero
        }
        
        int ffbo = strToTnt(fbo);
        int fi = -1;
        int h;
        for(h = 0; h< numCache; h++){
            if(strcmp(cache_array[h].tag, temp) == 0 && strcmp(cache_array[h].cacheIndex, fci) == 0){
                    fi = 0;
                
                    if(ffbo == 0){
                        printf("%s", cache_array[h].byte_offset0);
                        printf("\n");
                    }
                    else if(ffbo == 1){
                        printf("%s", cache_array[h].byte_offset1);
                        printf("\n");
                    }
                    else if(ffbo == 2){
                        printf("%s", cache_array[h].byte_offset2);
                        printf("\n");
                    }
                    else{
                        printf("%s", cache_array[h].byte_offset3);
                        printf("\n");
                    }
            }
        }
        if(fi == -1){
            printf("Can not be determined");
            printf("\n");
        }
    }
}
/**
 This is the main function. It is where the function begins executing. It reads in the file, called "input.txt", runs the
 readEntries function to read in all the data from the text file, and then asks the user three times to enter a virtual
 address. Once it has a virtual address, it will run the function mapVirtualToPhysicalAddress using what the user
 entered in binary form.
 */
int main() {
    
    //Reading input file
    FILE *input_file = fopen("input.txt", "r");
    
    // Read in TLB entries
    read_entries(input_file);
 
    // loop to get three hex values
    int i;
    for (i = 0; i < 3; i++) {
        
        //Getting User Input
        printf("Enter Virtual Address:");
        scanf("%s", hex);
    
        //Mapping
        map_virtual_to_physical_address(hexToBinary(hex));
    }
    return 0;
}
