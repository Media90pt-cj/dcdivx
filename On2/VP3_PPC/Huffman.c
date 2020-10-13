//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------


/****************************************************************************
*
*   Module Title :     Huffman.c
*
*   Description  :     Video CODEC
*
*
*****************************************************************************
*/

/****************************************************************************
*  Header Files
*****************************************************************************
*/

#define STRICT              /* Strict type checking. */

#include "SystemDependant.h"
#include "Huffman.h"
#include "pbdll.h"

#include "HuffTables.h"		

/****************************************************************************
*  Module constants.
*****************************************************************************
*/        

/****************************************************************************
*  Forward references.
*****************************************************************************
*/       
 
void BuildHuffmanTree( UINT32 RootIndex , UINT32 HIndex, UINT32 * FreqList );
                
/****************************************************************************
*  Exported Global Variables
*****************************************************************************
*/  

/****************************************************************************
*  Module Static Variables
*****************************************************************************
*/              

/****************************************************************************
 * 
 *  ROUTINE       :     SelectHuffmanSet
 *
 *  INPUTS        :     Encoder instance.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This function selects the huffman table set based on
 *						encoder version number
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void SelectHuffmanSet( PB_INSTANCE *pbi )
{
	UINT32 i;

    for ( i = 0; i < NUM_HUFF_TABLES; i++ )
	{
		pbi->HuffCodeArray_VP3x[i] = HuffCodeArray_VP31[i];
		pbi->HuffCodeLengthArray_VP3x[i] = HuffCodeLengthArray_VP31[i];
	}
	
	pbi->HuffRoot_VP3x = HuffRoot_VP31;
}

/****************************************************************************
 * 
 *  ROUTINE       :     CreateHuffmanTrees 
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This funtion creates the huffman trees.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void CreateHuffmanTrees() 
{    
	UINT32 i;

	// Destroy any existing / old trees
	DestroyHuffmanTrees();

	// Create huffman coding trees
	for ( i = 0; i < NUM_HUFF_TABLES; i++ ) 
	{
		// Tables for encoder versions <2
		BuildHuffmanTree( 0, i, FrequencyCounts1[i] );

		// Tables for encoder versions >=2
		//BuildHuffmanTree( 2, i, FrequencyCounts1[i] );
		BuildHuffmanTree( 2, i, FrequencyCounts2[i] );
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     DestroyHuffmanTrees
 *
 *  INPUTS        :     None.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This funtion destroys the huffman trees.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void DestroyHuffmanTrees()
{    
	UINT32 i;

	// Destroy huffman coding trees
	for ( i = 0; i < NUM_HUFF_TABLES; i++ ) 
	{
		// Tables for encoder versions < 2
		if ( HuffRoot_VP31[i] != NULL )
		{
			DestroyHuffTree( &HuffRoot_VP31[i] );
		}
		HuffRoot_VP31[i] = NULL;

		// Tables for encoder versions >= 2
		if ( HuffRoot_VP33[i] != NULL )
		{
			DestroyHuffTree( &HuffRoot_VP33[i] );
		}
		HuffRoot_VP33[i] = NULL;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     CreateHuffmanList
 *
 *  INPUTS        :     The tree root.
 *						The index for the new tree
 *                      A token frequency list.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This funtion creates the initial sorted huffman list.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void  CreateHuffmanList( HUFF_ENTRY ** HuffRoot, UINT32 HIndex, UINT32 * FreqList )
{    
    UINT32 i;
    HUFF_ENTRY * entry_ptr;
    HUFF_ENTRY * search_ptr;

    /* Create a HUFF entry for token zero. */
    HuffRoot[HIndex] = (HUFF_ENTRY *)malloc( sizeof(HUFF_ENTRY) );

    HuffRoot[HIndex]->Previous = NULL;
    HuffRoot[HIndex]->Next = NULL;
    HuffRoot[HIndex]->ZeroChild = NULL;
    HuffRoot[HIndex]->OneChild = NULL;
    HuffRoot[HIndex]->Value = 0;
    HuffRoot[HIndex]->Frequency = FreqList[0];   
    
    if ( HuffRoot[HIndex]->Frequency == 0 )
         HuffRoot[HIndex]->Frequency = 1; 
    
    /* Now add entries for all the other possible tokens. */
    for ( i = 1; i < MAX_ENTROPY_TOKENS; i++ )
    {
        //entry_ptr = GlobalAlloc( GPTR, sizeof(HUFF_ENTRY) );   
        entry_ptr = (HUFF_ENTRY *)malloc( sizeof(HUFF_ENTRY) );


        entry_ptr->Value = i;
        entry_ptr->Frequency = FreqList[i];
        entry_ptr->ZeroChild = NULL;
        entry_ptr->OneChild = NULL;

        /* Force min value of 1. This prevents the tree getting to deep. */
        if ( entry_ptr->Frequency == 0 )
            entry_ptr->Frequency = 1;
    
        if ( entry_ptr->Frequency <= HuffRoot[HIndex]->Frequency )
        {                                            
            entry_ptr->Next = (char * )HuffRoot[HIndex];  
            HuffRoot[HIndex]->Previous = (char * )entry_ptr;
            entry_ptr->Previous = (char * )NULL;   
            HuffRoot[HIndex] = entry_ptr;
        }
        else
        {  
            search_ptr = HuffRoot[HIndex];
            while ( (search_ptr->Next != NULL) && (search_ptr->Frequency < entry_ptr->Frequency) )
            {                                                                                  
                search_ptr = (HUFF_ENTRY *)search_ptr->Next;
            }
            
            if ( search_ptr->Frequency < entry_ptr->Frequency )
            {
                entry_ptr->Next = (char * )NULL;
                entry_ptr->Previous = (char * )search_ptr;   
                search_ptr->Next = (char * )entry_ptr;
            }
            else
            {
                entry_ptr->Next = (char * )search_ptr;
                entry_ptr->Previous = search_ptr->Previous; 
                ((HUFF_ENTRY *)(search_ptr->Previous))->Next = (char * )entry_ptr;
                search_ptr->Previous = (char * )entry_ptr;
            }
        }                                     
        
    }
    
    
}

/****************************************************************************
 * 
 *  ROUTINE       :     CreateCodeArray
 *
 *  INPUTS        :     The tree root
 *						A HuffCodeArray
 *						A HuffCodeLengthArray
 *                      The code value of the root.    
 *                      The length of the code in bits.
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This funtion creates the code array from the huffman tree
 *                      that is used to code tokens.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void CreateCodeArray( HUFF_ENTRY * HuffRoot, 
					  UINT32 * HuffCodeArray, 
					  UINT8  * HuffCodeLengthArray, 
					  UINT32  CodeValue, UINT8 CodeLength )
{    

    /* If we are at a leaf then fill in a code array entry. */
    if ( ( HuffRoot->ZeroChild == NULL ) && ( HuffRoot->OneChild == NULL ) )
    {                                                         
        HuffCodeArray[HuffRoot->Value] = CodeValue;
        HuffCodeLengthArray[HuffRoot->Value] = CodeLength;
    }

    else
    {
        /* Recursive calls to scan down the tree. */
        CreateCodeArray( (HUFF_ENTRY *)(HuffRoot->ZeroChild), HuffCodeArray, HuffCodeLengthArray, ((CodeValue << 1) + 0), (UINT8)(CodeLength + 1) );
        CreateCodeArray( (HUFF_ENTRY *)(HuffRoot->OneChild), HuffCodeArray, HuffCodeLengthArray, ((CodeValue << 1) + 1), (UINT8)(CodeLength + 1) );
    }
}

/****************************************************************************
 * 
 *  ROUTINE       :     BuildHuffmanTree
 *
 *  INPUTS        :     The root index.
 *						The index for the new tree
 *                      A token frequency list.
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This funtion creates the initial sorted huffman list.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

void  BuildHuffmanTree( UINT32 RootIndex, UINT32 HIndex, UINT32 * FreqList )
{   
    HUFF_ENTRY * entry_ptr;
    HUFF_ENTRY * search_ptr;
	HUFF_ENTRY ** HuffRoot;

	// Select the Huffmant tree root.
	if ( RootIndex < 2 )
		HuffRoot = HuffRoot_VP31;
	else
		HuffRoot = HuffRoot_VP33;

    /* First create a sorted linked list representing the frequencies of each token. */
    CreateHuffmanList( HuffRoot, HIndex, FreqList );
    
    /* Now build the tree from the list. */
     
    /* While there are at least two items left in the list. */ 
    while ( HuffRoot[HIndex]->Next != NULL )
    {                              
        /* Create the new node as the parent of the first two in the list. */
        //entry_ptr = GlobalAlloc( GPTR, sizeof(HUFF_ENTRY) );   
        entry_ptr = (HUFF_ENTRY *)malloc( sizeof(HUFF_ENTRY) );

        entry_ptr->Value = -1;
        entry_ptr->Frequency = HuffRoot[HIndex]->Frequency + ( ((HUFF_ENTRY *)HuffRoot[HIndex]->Next)->Frequency );
        entry_ptr->ZeroChild = (char *)HuffRoot[HIndex];
        entry_ptr->OneChild = (char *)(HuffRoot[HIndex]->Next);  
        
        /* If there are still more items in the list then insert the new node into the list. */  
        if ( ((HUFF_ENTRY *)entry_ptr->OneChild)->Next != NULL )
        {   
            /* Set up the provisional 'new root' */ 
            HuffRoot[HIndex] = (HUFF_ENTRY *)( ((HUFF_ENTRY *)entry_ptr->OneChild)->Next );
            HuffRoot[HIndex]->Previous = NULL;
            
            /* Now scan through the remaining list to insert the new entry at the appropriate point. */
            if ( entry_ptr->Frequency <= HuffRoot[HIndex]->Frequency )
            {                                            
                entry_ptr->Next = (char * )HuffRoot[HIndex];  
                HuffRoot[HIndex]->Previous = (char * )entry_ptr;
                entry_ptr->Previous = (char * )NULL;   
                HuffRoot[HIndex] = entry_ptr;
            }
            else
            {  
                search_ptr = HuffRoot[HIndex];
                while ( (search_ptr->Next != NULL) && 
                        (search_ptr->Frequency < entry_ptr->Frequency) )
                {                                                                                  
                    search_ptr = (HUFF_ENTRY *)search_ptr->Next;
                }
                
                if ( search_ptr->Frequency < entry_ptr->Frequency )
                {
                    entry_ptr->Next = (char * )NULL;
                    entry_ptr->Previous = (char * )search_ptr;   
                    search_ptr->Next = (char * )entry_ptr;
                }
                else
                {
                    entry_ptr->Next = (char * )search_ptr;
                    entry_ptr->Previous = search_ptr->Previous; 
                    ((HUFF_ENTRY *)(search_ptr->Previous))->Next = (char * )entry_ptr;
                    search_ptr->Previous = (char * )entry_ptr;
                }
            }                                     
        }                                                                                      
        else
        {   
            /* Build has finished. */ 
            entry_ptr->Next = NULL;
            entry_ptr->Previous = NULL;
            HuffRoot[HIndex] = entry_ptr;
        }
        
        /* Delete the Next/Previous properties of the children (PROB NOT NEC). */ 
        ((HUFF_ENTRY *)entry_ptr->ZeroChild)->Next = NULL;
        ((HUFF_ENTRY *)entry_ptr->ZeroChild)->Previous = NULL;
        ((HUFF_ENTRY *)entry_ptr->OneChild)->Next = NULL;
        ((HUFF_ENTRY *)entry_ptr->OneChild)->Previous = NULL;
        
    } 
    
    /* Now build a code array from the tree. */
	if ( RootIndex < 2 )
		CreateCodeArray( HuffRoot[HIndex], HuffCodeArray_VP31[HIndex], HuffCodeLengthArray_VP31[HIndex], 0, 0);
	else
		CreateCodeArray( HuffRoot[HIndex], HuffCodeArray_VP33[HIndex], HuffCodeLengthArray_VP33[HIndex], 0, 0);
}


/****************************************************************************
 * 
 *  ROUTINE       :     DestroyHuffTree
 *
 *  INPUTS        :     HUFF_ENTRY * * root_ptr
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     
 *
 *  FUNCTION      :     This funtion destroys a Huffman tree or sub-tree.
 *
 *  SPECIAL NOTES :     None. 
 *
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void  DestroyHuffTree( HUFF_ENTRY * * root_ptr)
{                                          
    if ( *root_ptr != NULL )
    {
        // Destroy the zero child sub tree. 
        if ( (*root_ptr)->ZeroChild != NULL )
        { 
            DestroyHuffTree((HUFF_ENTRY * *)&((*root_ptr)->ZeroChild));
        }
        
        // Destroy the one child sub tree. 
        if ( (*root_ptr)->OneChild != NULL )
        {
            DestroyHuffTree((HUFF_ENTRY * *)(&(*root_ptr)->OneChild));
        }    
        
        // Once all sub trees to this node are destroyed then free the memory for this node. 
        free( (char *) *root_ptr );
        *root_ptr = NULL;
    }
}



