typedef struct ActionT {
	char *Name;
	char *Value;
	unsigned short LastValue;
} ActionsType;

ActionsType SkillsName[50];

int uo_handle_updateskill(char *buf, int len); // 0x3A