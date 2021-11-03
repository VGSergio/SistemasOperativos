/**
 * Authors:
 * Vega García, Sergio
 * Seguí Vives, Mateo
 * Planells Torres, David
 **/

#include "my_lib.h"

/**
 * my_strlen computes the lenght of the given string without '\0'
 **/
size_t my_strlen(const char *str){
    size_t len = 0;
    int i = 0;
 
    while (str[i++]) {
        len++;
    }    
    return len;

}

/**
 * my_strcmp compares two given strings character by character
 * by their ASCII codes
 * < 0 means str1 < str2
 * > 0 means str2 < str1
 * = 0 means str1 = str2
 **/
int my_strcmp(const char *str1, const char *str2){
    int i=0;

    while(str1[i] && str2[i] && str1[i]==str2[i]){
        i++;
    }
    
    return str1[i]-str2[i];
}

/**
 * my_strcpy copies the string pointed by src onto dest
 **/
char *my_strcpy(char *dest, const char *src){
    int i = 0;

    while(src[i]){                                      
        dest[i] = src[i]; 
        i++;
    }
    dest[i] = '\0'; 

    return dest;
}

/**
 * my_strncpy copies the first n characters of the string pointed by src 
 * onto dest
 **/
char *my_strncpy(char *dest, const char *src, size_t n){
    memset(dest, 0, sizeof(*dest));
    int i = 0;
    
    while(src[i] && i<n){                                      
        dest[i] = src[i]; 
        i++;
    } 

    return dest;
}

/**
 * my_strcat appends the string src onto the string dest
 **/
char *my_strcat(char *dest, const char *src){
    int i=0;
    while(dest[i]){
        i++;
    }

    my_strcpy(&dest[i],src);

    return dest;
}

/**
 * my_strchr finds the first occurence of the char c in the string str.
 **/
char *my_strchr(const char *str, int c){
    
    while(*str && *str!=c){
        str++;
    }

    if(*str)
        return (char *) str;
    else
        return NULL;
}



void *my_stack_pop (struct my_stack *stack){
	
	struct my_stack_node* aux;

	if (stack -> top != NULL){
		aux=stack -> top;
		stack -> top= aux -> next;
		aux->next= NULL;
	}
	return aux;
}


int my_stack_len(struct my_stack *stack){

	size_t len = 0;
    int i = 0;
	
	struct my_stack_node* node;
    node = stack -> top;
	
	if (node != NULL){
		while (node) {
			len++;
			node=node -> next;
		}  
	}
	return len;
}
