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
