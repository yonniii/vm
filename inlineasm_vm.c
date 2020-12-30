#include <stdio.h>

#define MAX	22
#define S_MAX	4
#define R_MAX	4

struct code {
	int arr[MAX];
	int pc;
};

int readNext(struct code * _this) {
	if (_this->pc == MAX) 
		return -1;
	else{
		int tmp = _this->pc;
		_this->pc++;		
		return _this->arr[tmp];
	}
}

// 2. operand stack  (general, reusable)
struct mstack {
	int arr[S_MAX];
	int top;
} mst;


void push(struct mstack * _this, int data ) {
	if(_this->top+1 >= S_MAX) return;
	_this->arr[++_this->top] = data;
}

int pop(struct mstack * _this) {
	if(_this->top<0) return -1;
	int tmp = _this->arr[_this->top];
	_this->arr[_this->top--]=0;
	return tmp;
}


// 3. op code of VM (general, reusable)
#define  VPUSH	0
#define  VADD	1
#define  VSUB	2
#define  VMULT	3
#define  VASSGN	4
#define  VLOAD	5


// 4. vm engine (general, so reusable)
// for y+(x*z)
//-> push y push x push z mult add assgn w
// push 1 push 0 push 2 mult add assgn 3
// 0,1,2 are variables' id

void run_on_vm(struct code * bytes, int (* tvars)[] ) {

	int i = 0;
	int onebyte; 
	int arg1, arg2;
	int isCnst = 0;
	goto F;
	F:
        asm("FFF:");
        if ((onebyte = readNext(bytes)) == -1) return;

        if(onebyte == VPUSH){
            isCnst = VPUSH;
            goto CHECK;
        }else if(onebyte == VLOAD){
            isCnst = VLOAD;
            goto CHECK;
        }else if(onebyte == VADD){
            isCnst = VADD;
            goto ARG;
        }else if(onebyte == VSUB){
            isCnst = VSUB;
            goto ARG;
        }else if(onebyte == VMULT){
            isCnst = VMULT;
            goto ARG;
        }else if(onebyte == VASSGN){
            isCnst = VASSGN;
            goto ASSGN;
        }
	CHECK:
	    if( (arg1=readNext(bytes)) == -1) return;
        if(isCnst == VLOAD){
            goto LOAD;
        }else{
            goto PUSH;    
    ARG:
	    arg1 = pop(&mst);
    	arg2 = pop(&mst);
        if(isCnst == VADD){
            goto ADD;
        }else if(isCnst == VSUB){
            goto SUB;
        }else if(isCnst == VMULT){
            goto MULT;
        }	
    PUSH:
        asm(
            "mov %0,%%eax;"
            "add $0x1,%%eax;"
            "mov %%eax, %%edx;"
            "mov %1,%%eax;"
            "cmp %%edx, %%eax;"
            "jge CONT;"
            "leaveq;"
            "retq;"
            "CONT:"
            :
            :"r"(mst.top),"r"(S_MAX)            
        );
    	// if(mst.top+1 >= S_MAX) return;
        asm(
            "mov %0, %%eax;"
            "add $0x1,%%eax;"
            "mov %%eax, %0;"
            :
            :"m"(mst.top)
        );
        // top++

        // asm(
        //     "mov %0, %%eax;"
        //     "add $0x1,%%eax;"
        //     "mov %%eax, %0;"
        //     :"=g"(mst.arr[mst.top])
        
            // "lea    0x0(,%rax,4),%rcx;"
            // "lea    0x201763(%rip),%rax;"
            // "mov    -0x14(%rbp),%edx;"
            // "mov    %edx,(%rcx,%rax,1);"

        // );
        asm(
            "mov %1, %%ebx;"
            "mov %%ebx, %0;"
            :
            :"m"(mst.arr[mst.top]), "r"(arg1)
        );
	    // mst.arr[mst.top] = arg1;
        asm ("jmp FFF;");
        // goto F;
	LOAD:
        asm(
            "mov %0,%%eax;"
            "add $0x1,%%eax;"
            "mov %%eax, %%edx;"
            "mov %1,%%eax;"
            "cmp %%edx, %%eax;"
            "jge CONT2;"
            "leaveq;"
            "retq;"
            "CONT2:"
            :
            :"r"(mst.top),"r"(S_MAX)           
        );
    	// if(mst.top+1 >= S_MAX) return;
        
        asm(
            "mov %0, %%eax;"
            "add $0x1,%%eax;"
            "mov %%eax, %0;"
            :
            :"m"(mst.top)
        );
        int tmparg1 = (*tvars)[arg1];
        asm(
            "mov %1, %%ebx;"
            "mov %%ebx, %0;"
            :
            :"m"(mst.arr[mst.top]), "r"((*tvars)[arg1])
        );
	    // mst.arr[mst.top] = (*tvars)[arg1]; 
        asm ("jmp FFF;");   
        // goto F;
	ADD:
        push(&mst, arg2 + arg1);
        if ((onebyte = readNext(bytes)) == -1) return;
        if(onebyte == VPUSH){
            isCnst = VPUSH;
            goto CHECK;
        }else if(onebyte == VLOAD){
            isCnst = VLOAD;
            goto CHECK;
        }else if(onebyte == VADD){
            isCnst = VADD;
            goto ARG;
        }else if(onebyte == VSUB){
            isCnst = VSUB;
            goto ARG;
        }else if(onebyte == VMULT){
            isCnst = VMULT;
            goto ARG;
        }else if(onebyte == VASSGN){
            isCnst = VASSGN;
            goto ASSGN;
        }
        goto F;
	SUB:
        push(&mst, arg2 - arg1);
        goto F;
	MULT:
        push(&mst, arg2 * arg1);
        goto F;
	ASSGN:
	    arg1=readNext(bytes);
		if( (arg2=readNext(bytes)) == -1) return;
		(*tvars)[arg2] = pop(&mst);
        goto F;
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



int main()  {
 int result;
 int a = 10;
 int b = 4;
 int c = 2;
 int d = 5;
 struct code encoded_bytes; // VM  byte code 
int tempvars[R_MAX];			// mapping : real var => vm only var

begin_add(&encoded_bytes);
add(&encoded_bytes, VLOAD);
add(&encoded_bytes, 1);
 add(&encoded_bytes, VPUSH);
add(&encoded_bytes, 7);
 add(&encoded_bytes, VLOAD);
add(&encoded_bytes, 2);
 add(&encoded_bytes, VADD);
 add(&encoded_bytes, VMULT);
add(&encoded_bytes, VASSGN);
add(&encoded_bytes, VLOAD);
add(&encoded_bytes, 0);
;
add(&encoded_bytes, VLOAD);
add(&encoded_bytes, 2);
 add(&encoded_bytes, VLOAD);
add(&encoded_bytes, 0);
 add(&encoded_bytes, VADD);
 add(&encoded_bytes, VLOAD);
add(&encoded_bytes, 3);
 add(&encoded_bytes, VSUB);
add(&encoded_bytes, VASSGN);
add(&encoded_bytes, VLOAD);
add(&encoded_bytes, 1);
;
end_add(&encoded_bytes);
tempvars[0] = result; 
tempvars[1] = a; 
tempvars[2] = c; 
tempvars[3] = d; 
run_on_vm(&encoded_bytes, &tempvars); 
result = tempvars[0]; 
a = tempvars[1]; 
c = tempvars[2]; 
d = tempvars[3]; 
 printf("%d\n", a);
}


