# Report
> Project #1. Scanner 2020  
> 
> 2018000337  
> 장호우

## Environment
* Ubuntu 20.04.1 LTS
* flex 2.6.4
* gcc version 9.3.0

## Part I. Implementation of C-Scanner using C-code

## Part II. Implementation of C-Scanner using lex(flex) by Tiny.lmodification

## Example and Result Screenshot
Example: **test.1.txt**
```c
/* A program to perform Euclid's
   Algorithm to computer gcd */

int gcd (int u, int v)
{
	if (v == 0) return u;
	else return gcd(v,u-u/v*v);
	/* u-u/v*v == u mod v */
}

void main(void)
{
	int x; int y;
	x = input(); y = input();
	output(gcd(x,y));
}

```
Result Screenshot:

1. For `./scanner_cimpl`
	![](images/0001.jpg)
	![](images/0002.jpg)
	![](images/0003.jpg)

2. For `./scanner_flex`
	![](images/0004.jpg)
	![](images/0005.jpg)
	![](images/0006.jpg)


Example: **test.2.txt**
```c
void main(void)
{
	int i; int x[5];
	
	i = 0;
	while( i < 5 )
	{
		x[i] = input();

		i = i + 1;
	}

	i = 0;
	while( i <= 4 )
	{
		if( x[i] != 0 )
		{
			output(x[i]);
		}
	}
}

```
Result Screenshot:

1. For `./scanner_cimpl`
	![](images/0007.jpg)
	![](images/0008.jpg)
	![](images/0009.jpg)

2. For `./scanner_flex`
	![](images/00010.jpg)
	![](images/00011.jpg)
	![](images/00012.jpg)
