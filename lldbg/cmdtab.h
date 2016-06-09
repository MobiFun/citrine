/* this structure is used for interactive command dispatch */

struct cmdtab {
	char	*cmd;
	void	(*func)();
};
