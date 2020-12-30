// FROM: Original program
//
// #include <stdio.h>
// void test(){
//       
//	int x = 1;
//	int y = 100;
//	int z = 1000;
//	int w = x*(y+z); // 문장 하나 => vm 난독화 단위
//        
//	printf(w);
// }


// TO: Generated program
// simply put 0.~4. and generate 5.

#include <stdio.h>

// 0. for maximal values : to be updated!
#define MAX	100


// 1. code : array of bytes (general, reusable)
struct code {
	char arr[MAX];
	int pc;
};

char readNext(struct code * _this) {
	//return _this->arr[_this->pc];
	//_this->pc++;
	if (_this->pc == MAX) 
		return -1;
	else{
	//	printf("%d, %d\n",_this->pc, _this->arr[_this->pc]);
		int tmp = _this->pc;
		_this->pc++;		
		return _this->arr[tmp];

	}
}

// 2. operand stack  (general, reusable)
struct mstack {
	int arr[MAX];
	int top;
} mst;


void push(struct mstack * _this, int data ) {
	_this->arr[_this->top] = data;
	// printf("push %d, at %d\n",_this->arr[_this->top],_this->top);
	_this->top++;
}

int pop(struct mstack * _this) {
	//_this->top--;
	int tmp = _this->arr[--_this->top];
	_this->arr[_this->top]=0;
	// printf("pop %d\n",_this->top-1);
	return tmp;
}


// 3. op code of VM (general, reusable)
#define  VPUSH	-1
#define  VADD	-2 
#define  VSUB	-3
#define  VMULT	-4
#define  VASSGN	-5


// 4. vm engine (general, so reusable)
// for y+(x*z)
//-> push y push x push z mult add assgn w
// push 1 push 0 push 2 mult add assgn 3
// 0,1,2 are variables' id

void run_on_vm(struct code * bytes, int (* tvars)[] ) {

	int i = 0;
	char onebyte; 
	int arg1, arg2;
	// printf("run_on_vm\n");
	while( (onebyte = readNext(bytes)) != 0) {
		// printf("%d\n", onebyte);
		switch (onebyte) {
			case VPUSH :
				if( (arg1=readNext(bytes)) == -1) return;
				//printf("push %d\n",(*tvars)[args1]);
				push(&mst, (*tvars)[arg1]);
				break;
			case VADD:
				arg1 = pop(&mst);
				arg2 = pop(&mst);
				// printf("add %d %d\n",arg1, arg2);
				push(&mst, arg1 + arg2);
				break;
			case VSUB:
				arg1 = pop(&mst);
				arg2 = pop(&mst);
				push(&mst, arg1-arg2);
				break;
			case VMULT:
				arg1 = pop(&mst);
				arg2 = pop(&mst);
				// printf("mult %d %d\n",arg1, arg2);
				push(&mst, arg1*arg2);
				break;
			case VASSGN:
				if( (arg1=readNext(bytes)) == -1) return;
				(*tvars)[arg1] = pop(&mst);
				break;
		} 
	}

}

void add(struct code * bytes, int op){
	bytes->arr[bytes->pc++] = op;
}

void begin_add(struct code * bytes){
	bytes->pc = 0;
	for(int i = 0; i<MAX ; i++){
		bytes->arr[i] = 0;		
	}
	// *mst.top = 0;
	mst.top = 0;
}

void end_add(struct code * bytes){
	bytes->pc = 0;
}



// 5. main (not general, should be generated for each assignment in original C program)
int main(){

	int x = 1;
	int y = 10;
	int z = 100;
	int w=0;

	// begin Obf

	// -> push y push x push z mult add assgn w
	// -> push 1 push 0 push 2 mult add assgn 3
	// where 0,1,2,3 are variables' id's

	struct code encoded_bytes; 	// VM  byte code 
	int tempvars[MAX];			// mapping : real var => vm only var

	// code generation
	begin_add(&encoded_bytes);
	add(&encoded_bytes, VPUSH);
	add(&encoded_bytes, 0);
	add(&encoded_bytes, VPUSH);
	add(&encoded_bytes, 1);
	add(&encoded_bytes, VPUSH);
	add(&encoded_bytes, 2);
	add(&encoded_bytes, VMULT);
	add(&encoded_bytes, VADD);
	add(&encoded_bytes, VASSGN);
	add(&encoded_bytes, 3);
	end_add(&encoded_bytes);

	// prolog
	// variable mapping from/to outside the vm
	tempvars[0] = x;
	tempvars[1] = y;
	tempvars[2] = z;
	tempvars[3] = w; // will be rewritten;


	// to run obfuscated x * (y+z) on VM
	run_on_vm(&encoded_bytes, &tempvars);

	// epilogue
	// variable mapping from/to outside the vm
	x = tempvars[0];
	y = tempvars[1];
	z = tempvars[2];
	w = tempvars[3];

	// end of Obf

	printf("%d\n",w);
}
