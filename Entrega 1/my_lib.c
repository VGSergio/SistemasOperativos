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
size_t my_strlen(const char *str)
{
    size_t len = 0;
    int i = 0;

    while (str[i++])
    {
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
int my_strcmp(const char *str1, const char *str2)
{
    int i = 0;

    while (str1[i] && str2[i] && str1[i] == str2[i])
    {
        i++;
    }

    return str1[i] - str2[i];
}

/**
 * my_strcpy copies the string pointed by src onto dest
 **/
char *my_strcpy(char *dest, const char *src)
{
    int i = 0;

    while (src[i])
    {
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
char *my_strncpy(char *dest, const char *src, size_t n)
{
    memset(dest, 0, sizeof(*dest));
    int i = 0;

    while (src[i] && i < n)
    {
        dest[i] = src[i];
        i++;
    }

    return dest;
}

/**
 * my_strcat appends the string src onto the string dest
 **/
char *my_strcat(char *dest, const char *src)
{
    int i = 0;
    while (dest[i])
    {
        i++;
    }

    my_strcpy(&dest[i], src);

    return dest;
}

/**
 * my_strchr finds the first occurence of the char c in the string str.
 **/
char *my_strchr(const char *str, int c)
{

    while (*str && *str != c)
    {
        str++;
    }

    if (*str)
        return (char *)str;
    else
        return NULL;
}

struct my_stack *my_stack_init(int size)
{
    struct my_stack *stack;
    stack = malloc(sizeof(struct my_stack));
    if (stack == NULL)
    {
        return NULL;
    }
    stack->size = size;
    stack->top = NULL;
    return stack;
}

int my_stack_push(struct my_stack *stack, void *data)
{
    if (stack == NULL || stack->size <= 0)
    {
        return EXIT_FAILURE;
    }
    struct my_stack_node *nuevo = malloc(sizeof(struct my_stack_node));
    if (nuevo == NULL)
    {
        return EXIT_FAILURE;
    }
    nuevo->data = data;
    nuevo->next = stack->top;
    stack->top = nuevo;
    return EXIT_SUCCESS;
}


void *my_stack_pop(struct my_stack *stack)
{

    if (stack == NULL || stack->top == NULL)
    {
        return NULL;
    }

    struct my_stack_node *aux;

    aux = stack->top;
    stack->top = aux->next;
    aux->next = NULL;

    return aux->data;
}

int my_stack_len(struct my_stack *stack)
{

    size_t len = 0;

    struct my_stack_node *node;
    node = stack->top;

    if (node != NULL)
    {
        while (node)
        {
            len++;
            node = node->next;
        }
    }
    return len;
}
int borrado(struct my_stack_node *elemento, int data_size)
{

    if (elemento && elemento->next == NULL)
    {
        free(elemento->data);
        free(elemento);

        return sizeof(struct my_stack_node) + data_size;
    }
    else
    {

        int bytes = borrado(elemento->next, data_size) + sizeof(struct my_stack_node) + data_size;
        //Liberar y sumar.
        free(elemento->data);
        free(elemento);
        return bytes;
    }
}

int my_stack_purge(struct my_stack *stack)
{
    int bytes = 0;
    if (stack != NULL && stack->top)
    {
        bytes = borrado(stack->top, stack->size);

        
    }
    free(stack);
    bytes += sizeof(struct my_stack);

    return bytes;
}

int escritura_fichero(struct my_stack_node *elemento, int fd, size_t size)
{
    ssize_t longitud;

    if (elemento->next == NULL)
    {
        //Escribimos el tamaño de los datos
        if ((longitud = write(fd, &size, sizeof(int))) == 0)
        {
            return 0;
        }
        //Escribir elemento.
        if ((longitud = write(fd, elemento->data, size)) == 0)
        {
            return 0;
        }

        return 1;
    }
    else
    {

        int elementos = escritura_fichero(elemento->next, fd, size) + 1;
        //Escribir.
        longitud = write(fd, elemento->data, size);
        return elementos;
    }
}

int my_stack_write(struct my_stack *stack, char *filename)
{
    if (stack == NULL || stack->top == NULL)
    {
        return -1;
    }

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        return -1;
    }
    int elementos = escritura_fichero(stack->top, fd, stack->size);
    int cerrar = close(fd);
    if (cerrar == -1)
    {
        return -1;
    }
    return elementos;
}

struct my_stack *my_stack_read(char *filename)
{

    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        return NULL;
    }
    void *data = malloc(sizeof(int));
    if (data == NULL)
    {
        return NULL;
    }
    ssize_t leido = read(fd, data, sizeof(int));

    struct my_stack *stack;
    int size = *(int *)data;

    stack = my_stack_init(size);
    free(data);
    data = malloc(stack->size);
    if (data == NULL)
    {
        return NULL;
    }
    while ((leido = read(fd, data, stack->size)) != 0)
    {
        my_stack_push(stack, data);
        data = malloc(stack->size);
        if (data == NULL)
        {
            return NULL;
        }
    }
    free(data);
    int cerrar = close(fd);
    if (cerrar == -1)
    {
        return NULL;
    }
    return stack;
}
