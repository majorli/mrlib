unsigned int lg2(unsigned int n)
{
	unsigned int ret = 0;
	unsigned int tmp = n;
	while ((tmp = tmp >> 1) > 0)
		ret++;
	return ret;
}
