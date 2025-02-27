//! \file
/*
**  Copyright (C) - Triton
**
**  This program is under the terms of the BSD License.
*/

#include <cstring>

#include <exceptions.hpp>
#include <immediate.hpp>
#include <instruction.hpp>



namespace triton {
  namespace arch {

    Instruction::Instruction() {
      this->address         = 0;
      this->branch          = false;
      this->conditionTaken  = false;
      this->controlFlow     = false;
      this->prefix          = 0;
      this->size            = 0;
      this->tainted         = false;
      this->tid             = 0;
      this->type            = 0;

      std::memset(this->opcodes, 0x00, sizeof(this->opcodes));
    }


    Instruction::Instruction(const triton::uint8* opcodes, triton::uint32 opSize) : Instruction::Instruction() {
      this->setOpcodes(opcodes, opSize);
    }


    Instruction::~Instruction() {
    }


    Instruction::Instruction(const Instruction& other) {
      this->copy(other);
    }


    void Instruction::operator=(const Instruction& other) {
      this->copy(other);
    }


    void Instruction::copy(const Instruction& other) {
      this->address             = other.address;
      this->branch              = other.branch;
      this->conditionTaken      = other.conditionTaken;
      this->controlFlow         = other.controlFlow;
      this->loadAccess          = other.loadAccess;
      this->memoryAccess        = other.memoryAccess;
      this->operands            = other.operands;
      this->prefix              = other.prefix;
      this->registerState       = other.registerState;
      this->size                = other.size;
      this->storeAccess         = other.storeAccess;
      this->symbolicExpressions = other.symbolicExpressions;
      this->tainted             = other.tainted;
      this->tid                 = other.tid;
      this->type                = other.type;

      std::memcpy(this->opcodes, other.opcodes, sizeof(this->opcodes));

      this->disassembly.clear();
      this->disassembly.str(other.disassembly.str());
    }


    triton::uint32 Instruction::getThreadId(void) const {
      return this->tid;
    }


    void Instruction::setThreadId(triton::uint32 tid) {
      this->tid = tid;
    }


    triton::uint64 Instruction::getAddress(void) const {
      return this->address;
    }


    triton::uint64 Instruction::getNextAddress(void) const {
      return this->address + this->size;
    }


    void Instruction::setAddress(triton::uint64 addr) {
      this->address = addr;
    }


    std::string Instruction::getDisassembly(void) const {
      return this->disassembly.str();
    }


    const triton::uint8* Instruction::getOpcodes(void) const {
      return this->opcodes;
    }


    void Instruction::setOpcodes(const triton::uint8* opcodes, triton::uint32 size) {
      if (size >= sizeof(this->opcodes))
       throw triton::exceptions::Instruction("Instruction::setOpcodes(): Invalid size (too big).");
      std::memcpy(this->opcodes, opcodes, size);
      this->size = size;
    }


    triton::uint32 Instruction::getSize(void) const {
      return this->size;
    }


    triton::uint32 Instruction::getType(void) const {
      return this->type;
    }


    triton::uint32 Instruction::getPrefix(void) const {
      return this->prefix;
    }


    const std::set<std::pair<triton::arch::MemoryAccess, triton::ast::AbstractNode*>>& Instruction::getLoadAccess(void) const {
      return this->loadAccess;
    }


    const std::set<std::pair<triton::arch::MemoryAccess, triton::ast::AbstractNode*>>& Instruction::getStoreAccess(void) const {
      return this->storeAccess;
    }


    const std::set<std::pair<triton::arch::Register, triton::ast::AbstractNode*>>& Instruction::getReadRegisters(void) const {
      return this->readRegisters;
    }


    const std::set<std::pair<triton::arch::Register, triton::ast::AbstractNode*>>& Instruction::getWrittenRegisters(void) const {
      return this->writtenRegisters;
    }


    const std::set<std::pair<triton::arch::Immediate, triton::ast::AbstractNode*>>& Instruction::getReadImmediates(void) const {
      return this->readImmediates;
    }


    void Instruction::updateContext(const triton::arch::MemoryAccess& mem) {
      this->memoryAccess.push_back(mem);
    }


    /* If there is a concrete value recorded, build the appropriate Register. Otherwise, perfrom the analysis on zero. */
    triton::arch::Register Instruction::getRegisterState(triton::uint32 regId) {
      triton::arch::Register reg(regId);
      if (this->registerState.find(regId) != this->registerState.end())
        reg = this->registerState[regId];
      return reg;
    }


    void Instruction::setLoadAccess(const triton::arch::MemoryAccess& mem, triton::ast::AbstractNode* node) {
      this->loadAccess.insert(std::make_pair(mem, node));
    }


    void Instruction::setStoreAccess(const triton::arch::MemoryAccess& mem, triton::ast::AbstractNode* node) {
      this->storeAccess.insert(std::make_pair(mem, node));
    }


    void Instruction::setReadRegister(const triton::arch::Register& reg, triton::ast::AbstractNode* node) {
      this->readRegisters.insert(std::make_pair(reg, node));
    }


    void Instruction::setWrittenRegister(const triton::arch::Register& reg, triton::ast::AbstractNode* node) {
      this->writtenRegisters.insert(std::make_pair(reg, node));
    }


    void Instruction::setReadImmediate(const triton::arch::Immediate& imm, triton::ast::AbstractNode* node) {
      this->readImmediates.insert(std::make_pair(imm, node));
    }


    void Instruction::setSize(triton::uint32 size) {
      this->size = size;
    }


    void Instruction::setType(triton::uint32 type) {
      this->type = type;
    }


    void Instruction::setPrefix(triton::uint32 prefix) {
      this->prefix = prefix;
    }


    void Instruction::setDisassembly(const std::string& str) {
      this->disassembly.clear();
      this->disassembly.str(str);
    }


    void Instruction::setTaint(void) {
      std::vector<triton::engines::symbolic::SymbolicExpression*>::const_iterator it;
      for (it = this->symbolicExpressions.begin(); it != this->symbolicExpressions.end(); it++) {
        if ((*it)->isTainted == true) {
          this->tainted = true;
          break;
        }
      }
    }


    void Instruction::updateContext(const triton::arch::Register& reg) {
      this->registerState[reg.getId()] = reg;
    }


    void Instruction::addSymbolicExpression(triton::engines::symbolic::SymbolicExpression* expr) {
      if (expr == nullptr)
        throw triton::exceptions::Instruction("Instruction::addSymbolicExpression(): Cannot add a null expression.");
      this->symbolicExpressions.push_back(expr);
    }


    bool Instruction::isBranch(void) const {
      return this->branch;
    }


    bool Instruction::isControlFlow(void) const {
      return this->controlFlow;
    }


    bool Instruction::isConditionTaken(void) const {
      return this->conditionTaken;
    }


    bool Instruction::isTainted(void) const {
      return this->tainted;
    }


    bool Instruction::isSymbolized(void) const {
      std::vector<triton::engines::symbolic::SymbolicExpression*>::const_iterator it;
      for (it = this->symbolicExpressions.begin(); it != this->symbolicExpressions.end(); it++) {
        if ((*it)->isSymbolized() == true)
          return true;
      }
      return false;
    }


    bool Instruction::isMemoryRead(void) const {
      if (this->loadAccess.size() >= 1)
        return true;
      return false;
    }


    bool Instruction::isMemoryWrite(void) const {
      if (this->storeAccess.size() >= 1)
        return true;
      return false;
    }


    bool Instruction::isPrefixed(void) const {
      if (this->prefix)
        return true;
      return false;
    }


    void Instruction::setBranch(bool flag) {
      this->branch = flag;
    }


    void Instruction::setControlFlow(bool flag) {
      this->controlFlow = flag;
    }


    void Instruction::setConditionTaken(bool flag) {
      this->conditionTaken = flag;
    }


    void Instruction::reset(void) {
      this->partialReset();
      this->memoryAccess.clear();
      this->registerState.clear();
    }


    void Instruction::partialReset(void) {
      this->address         = 0;
      this->branch          = false;
      this->conditionTaken  = false;
      this->controlFlow     = false;
      this->size            = 0;
      this->tainted         = false;
      this->tid             = 0;
      this->type            = 0;

      this->disassembly.clear();
      this->loadAccess.clear();
      this->operands.clear();
      this->readImmediates.clear();
      this->readRegisters.clear();
      this->storeAccess.clear();
      this->symbolicExpressions.clear();
      this->writtenRegisters.clear();

      std::memset(this->opcodes, 0x00, sizeof(this->opcodes));
    }


    std::ostream& operator<<(std::ostream& stream, const Instruction& inst) {
      stream << std::hex << inst.getAddress() << ": " << inst.getDisassembly() << std::dec;
      return stream;
    }


    std::ostream& operator<<(std::ostream& stream, const Instruction* inst) {
      stream << *inst;
      return stream;
    }

  };
};

