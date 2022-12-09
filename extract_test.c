#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

int	main()
{

	char	*ptr = calloc(1, sizeof(char) * 1000);
	int	fd = open("./test_message.txt", O_RDONLY);
	if (ptr == NULL || fd < 0)
	{
		printf("error with malloc or open\n");
	}
	char	*ptr_copy = ptr;
	printf("BEFORE READ address of ptr : %p\n", ptr);
	printf("BEFORE READ address of ptr_copy : %p\n", ptr_copy);
	int	read_return = read(fd, ptr, 999);
	printf("AFTER READ address of ptr : %p\n", ptr);
	printf("AFTER READ address of ptr_copy : %p\n", ptr_copy);
	char	*msg = NULL;
	char	*to_add = "blabla : ";

	while (extract_message(&ptr, &msg))
	{
		printf("INSIDE WHILE address of ptr : %p\n", ptr);
		printf("INSIDE WHILE address of ptr_copy : %p\n", ptr_copy);
		char	*temp = str_join(NULL, to_add);
		temp = str_join(temp, msg);
		printf("string extracted : %s\n", temp);
		free(temp); // was allocated by str_join()
		free(msg); // was allocated by calloc in here
	}

	printf("AFTER WHILE address of ptr : %p\n", ptr);
	printf("AFTER WHILE address of ptr_copy : %p\n", ptr_copy);

	free(ptr); // was allocated by extract_message()

	//system("leaks a.out");

	return 0;
}
