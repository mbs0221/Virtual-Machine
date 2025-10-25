#include "tac.h"
#include "../Asm/inter.h"
#include <cstdio>

// 从Asm的Code列表构造TAC程序的实现
TACProgram::TACProgram(const list<Code*>& codes) {
    // 从Asm的Code列表构造TAC程序
    // 这里需要将Code转换为TAC指令
    // 暂时使用TACConverter的现有逻辑
    TACConverter converter;
    TACProgram* result = converter.convertFromAST(codes);
    if (result) {
        // 复制指令
        const vector<TACInstruction>& resultInstructions = result->getInstructions();
        for (size_t i = 0; i < resultInstructions.size(); i++) {
            addInstruction(resultInstructions[i]);
        }
    }
}

// TACConverter::convertFromAST的实现
TACProgram* TACConverter::convertFromAST(const list<Code*>& codes) {
	if (tac) {
		delete tac;
	}
	tac = new TACProgram();
	
	list<Code*>::const_iterator iter;
	for (iter = codes.begin(); iter != codes.end(); iter++) {
		Code* code = *iter;
		TACInstruction inst(TAC_HALT, code->line);
		
		switch ((int)code->opt) {
			case (int)DATA: {
				// data指令转换为注释或忽略
				break;
			}
			case (int)VAR: {
				Variable* var = (Variable*)code;
				// 变量声明转换为赋值
				inst.op = TAC_ASSIGN;
				inst.result = TACOperand(TAC_TEMP, var->name);
				inst.arg1 = TACOperand(TAC_IMM, var->value);
				break;
			}
			case LOAD: {
				Load* load = (Load*)code;
				inst.op = TAC_LOAD;
				inst.result = TACOperand(TAC_REG, load->reg);
				inst.arg1 = TACOperand(TAC_MEM, load->addr);
				break;
			}
			case STORE: {
				Store* store = (Store*)code;
				inst.op = TAC_STORE;
				inst.result = TACOperand(TAC_MEM, store->addr);
				inst.arg1 = TACOperand(TAC_REG, store->reg);
				break;
			}
			case ADD: {
				Arith* arith = (Arith*)code;
				inst.op = TAC_ADD;
				inst.result = TACOperand(TAC_REG, arith->reg1);
				inst.arg1 = TACOperand(TAC_REG, arith->reg2);
				inst.arg2 = TACOperand(TAC_REG, arith->reg3);
				break;
			}
			case SUB: {
				Arith* arith = (Arith*)code;
				inst.op = TAC_SUB;
				inst.result = TACOperand(TAC_REG, arith->reg1);
				inst.arg1 = TACOperand(TAC_REG, arith->reg2);
				inst.arg2 = TACOperand(TAC_REG, arith->reg3);
				break;
			}
			case MUL: {
				Arith* arith = (Arith*)code;
				inst.op = TAC_MUL;
				inst.result = TACOperand(TAC_REG, arith->reg1);
				inst.arg1 = TACOperand(TAC_REG, arith->reg2);
				inst.arg2 = TACOperand(TAC_REG, arith->reg3);
				break;
			}
			case DIV: {
				Arith* arith = (Arith*)code;
				inst.op = TAC_DIV;
				inst.result = TACOperand(TAC_REG, arith->reg1);
				inst.arg1 = TACOperand(TAC_REG, arith->reg2);
				inst.arg2 = TACOperand(TAC_REG, arith->reg3);
				break;
			}
			case MOD: {
				Arith* arith = (Arith*)code;
				inst.op = TAC_MOD;
				inst.result = TACOperand(TAC_REG, arith->reg1);
				inst.arg1 = TACOperand(TAC_REG, arith->reg2);
				inst.arg2 = TACOperand(TAC_REG, arith->reg3);
				break;
			}
			case CALL: {
				Call* call = (Call*)code;
				inst.op = TAC_CALL;
				inst.result = TACOperand(TAC_REG, (BYTE)0); // 返回值寄存器
				inst.label = call->addr->w->word; // 从Label获取标签名
				break;
			}
			case RET: {
				inst.op = TAC_RETURN;
				inst.arg1 = TACOperand(TAC_REG, (BYTE)0); // 返回值寄存器
				break;
			}
			case JMP: {
				Jmp* jmp = (Jmp*)code;
				inst.op = TAC_JUMP;
				inst.label = jmp->addr->w->word; // 从Label获取标签名
				break;
			}
			case PUSH: {
				Push* push = (Push*)code;
				inst.op = TAC_PUSH;
				inst.arg1 = TACOperand(TAC_REG, push->reg);
				break;
			}
			case POP: {
				Pop* pop = (Pop*)code;
				inst.op = TAC_POP;
				inst.result = TACOperand(TAC_REG, pop->reg);
				break;
			}
			case MOV: {
				Mov* mov = (Mov*)code;
				inst.op = TAC_MOV;
				inst.result = TACOperand(TAC_REG, mov->reg1);
				inst.arg1 = TACOperand(TAC_REG, mov->reg2);
				break;
			}
			default: {
				// 其他指令暂时忽略
				break;
			}
		}
		
		// 处理标签
		if (code->opt == 0) { // 标签
			LabelCode* label = (LabelCode*)code;
			inst.op = TAC_LABEL;
			inst.label = label->labelName;
		}
		
		tac->addInstruction(inst);
	}
	
	return tac;
}
