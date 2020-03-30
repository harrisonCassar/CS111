#include <stdio.h>
#include <string.h>  // for strlen
#include <assert.h>
#include <zlib.h>

#define BUFSIZE 256

// adapted from: http://stackoverflow.com/questions/7540259/deflate-and-inflate-zlib-h-in-c
int main(int argc, char* argv[])
{   
    char input[256] = "abcdabcdabcdabcz";
    char output[256];
    char decomp[256];

    printf("Uncompressed size is: %lu\n", strlen(input));
    printf("Uncompressed string is: %s\n", input);

    printf("\n----------\n\n");
    
    // zlib struct
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    
    //init deflate
    int ret = deflateInit(&defstream, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK)
    	return 1;
	
	//read-in input
    int bytesRead = 16;//read(0,input,256);

	//prepare for deflation
    defstream.avail_in = (uInt) bytesRead;
    defstream.next_in = (Bytef *) input;
    defstream.avail_out = (uInt) BUFSIZE;
    defstream.next_out = (Bytef *) output;

    printf("%d\n",(int) defstream.avail_in);
    while (defstream.avail_in != 0)
    {	
    	defstream.avail_out = (uInt) BUFSIZE;
    	defstream.next_out = (Bytef *) output;
    	
    	printf("Ran through loop!\n");
    	ret = deflate(&defstream, Z_SYNC_FLUSH);
		if (ret != Z_OK)
    		return 1;

    	//send off output stream
    	if (defstream.avail_out == 0)
    		write(1,output,BUFSIZE);
    	else
    		write(1,output,BUFSIZE-defstream.avail_out);
    }
    //sent all input
     
    // This is one way of getting the size of the output
    printf("Compressed size is: %lu\n", defstream.total_out);
    printf("Compressed string is: %s\n", output);
    

    printf("\n----------\n\n");

    // STEP 2.
    // inflate b into c
    // zlib struct
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    // setup "b" as the input and "c" as the compressed output
    
    //prepare for inflation
    infstream.avail_in = (uInt) ((char*)defstream.next_out - output); // size of input
    infstream.next_in = (Bytef *) output; // input char array
    //infstream.avail_out = (uInt) BUFSIZE; // size of output
    //infstream.next_out = (Bytef *) decomp; // output char array
     
    // the actual DE-compression work.
    ret = inflateInit(&infstream);
    if (ret != Z_OK)
    	return 1;

    printf("%d\n",(int) infstream.avail_in);
    while (infstream.avail_in != 0)
    {
    	infstream.avail_out = (uInt) BUFSIZE;
    	infstream.next_out = (Bytef *) decomp;

    	printf("Ran through loop!\n");
    	ret = inflate(&infstream, Z_SYNC_FLUSH);
		if (ret != Z_OK)
    		return 1;

    	//send off output stream
    	if (infstream.avail_out == 0)
    		write(1,output,BUFSIZE);
    	else
    		write(1,output,BUFSIZE-infstream.avail_out);
    }

    deflateEnd(&defstream);
    inflateEnd(&infstream);
     
    printf("Uncompressed size is: %lu\n", strlen(decomp));
    printf("Uncompressed string is: %s\n", decomp);

    return 0;
}