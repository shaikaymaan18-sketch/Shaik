// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/arm/nce/visitor_base.h"

#include <algorithm>
#include <optional>

namespace oaknut {
struct XReg;
}

namespace Core {

std::optional<oaknut::XReg> CheckForPlatformRegister(u32 instruction);

class PlatformVisitor final : public VisitorBase {
public:
    PlatformVisitor();
    ~PlatformVisitor() override = default;

    Reg scratch;

    template<typename... Regs>
    void ChooseScratch(Regs... args) {
        static_assert((std::is_same_v<Regs, Reg> && ...));
        std::array<Reg, sizeof...(args)> regs{args...};

        for (int i = 0; i < 31; ++i) {
            if (std::find(regs.begin(), regs.end(), static_cast<Reg>(i)) == regs.end()) {
                scratch = static_cast<Reg>(i);
                return;
            }
        }

        UNREACHABLE();
    }

    bool UnallocatedEncoding() override {
        return false;
    }

    bool ADR(Imm<2> immlo, Imm<19> immhi, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool ADRP(Imm<2> immlo, Imm<19> immhi, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool ADDG(Imm<6> offset_imm, Imm<4> tag_offset, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SUBG(Imm<6> offset_imm, Imm<4> tag_offset, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool ADD_imm(bool sf, Imm<2> shift, Imm<12> imm12, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool ADDS_imm(bool sf, Imm<2> shift, Imm<12> imm12, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SUB_imm(bool sf, Imm<2> shift, Imm<12> imm12, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SUBS_imm(bool sf, Imm<2> shift, Imm<12> imm12, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool AND_imm(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool ORR_imm(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool EOR_imm(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool ANDS_imm(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool MOVN(bool sf, Imm<2> hw, Imm<16> imm16, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool MOVZ(bool sf, Imm<2> hw, Imm<16> imm16, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool MOVK(bool sf, Imm<2> hw, Imm<16> imm16, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool SBFM(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool BFM(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool UBFM(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool ASR_1(Imm<5> immr, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool ASR_2(Imm<6> immr, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SXTB_1(Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SXTB_2(Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SXTH_1(Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SXTH_2(Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SXTW(Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool EXTR(bool sf, bool N, Reg Rm, Imm<6> imms, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool XPAC_1(bool D, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool PACIA_1(bool Z, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool PACIB_1(bool Z, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool AUTIA_1(bool Z, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool AUTIB_1(bool Z, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool SYS(Imm<3> op1, Imm<4> CRn, Imm<4> CRm, Imm<3> op2, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool MSR_reg(Imm<1> o0, Imm<3> op1, Imm<4> CRn, Imm<4> CRm, Imm<3> op2, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool SYSL(Imm<3> op1, Imm<4> CRn, Imm<4> CRm, Imm<3> op2, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool MRS(Imm<1> o0, Imm<3> op1, Imm<4> CRn, Imm<4> CRm, Imm<3> op2, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool RMIF(Imm<6> lsb, Reg Rn, Imm<4> mask) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool SETF8(Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool SETF16(Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool DC_IVAC(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool DC_ISW(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool DC_CSW(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool DC_CISW(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool DC_ZVA(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool DC_CVAC(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool DC_CVAU(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool DC_CVAP(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool DC_CIVAC(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool IC_IVAU(Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool BR(Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool BRA(bool Z, bool M, Reg Rn, Reg Rm) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool BLR(Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool BLRA(bool Z, bool M, Reg Rn, Reg Rm) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool RET(Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool CBZ(bool sf, Imm<19> imm19, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool CBNZ(bool sf, Imm<19> imm19, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool TBZ(Imm<1> b5, Imm<5> b40, Imm<14> imm14, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool TBNZ(Imm<1> b5, Imm<5> b40, Imm<14> imm14, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool STx_mult_1(bool Q, Imm<4> opcode, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STx_mult_2(bool Q, Reg Rm, Imm<4> opcode, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LDx_mult_1(bool Q, Imm<4> opcode, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LDx_mult_2(bool Q, Reg Rm, Imm<4> opcode, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool ST1_sngl_1(bool Q, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool ST1_sngl_2(bool Q, Reg Rm, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool ST3_sngl_1(bool Q, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool ST3_sngl_2(bool Q, Reg Rm, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool ST2_sngl_1(bool Q, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool ST2_sngl_2(bool Q, Reg Rm, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool ST4_sngl_1(bool Q, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool ST4_sngl_2(bool Q, Reg Rm, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LD1_sngl_1(bool Q, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LD1_sngl_2(bool Q, Reg Rm, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LD3_sngl_1(bool Q, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LD3_sngl_2(bool Q, Reg Rm, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LD1R_1(bool Q, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LD1R_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LD3R_1(bool Q, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LD3R_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LD2_sngl_1(bool Q, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LD2_sngl_2(bool Q, Reg Rm, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LD4_sngl_1(bool Q, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LD4_sngl_2(bool Q, Reg Rm, Imm<2> upper_opcode, bool S, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LD2R_1(bool Q, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LD2R_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LD4R_1(bool Q, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LD4R_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool STXR(Imm<2> size, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool STLXR(Imm<2> size, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool STXP(Imm<1> size, Reg Rs, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool STLXP(Imm<1> size, Reg Rs, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDXR(Imm<2> size, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDAXR(Imm<2> size, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDXP(Imm<1> size, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDAXP(Imm<1> size, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STLLR(Imm<2> size, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STLR(Imm<2> size, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDLAR(Imm<2> size, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDAR(Imm<2> size, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool CASP(bool sz, bool L, Reg Rs, bool o0, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool CASB(bool L, Reg Rs, bool o0, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool CASH(bool L, Reg Rs, bool o0, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool CAS(bool sz, bool L, Reg Rs, bool o0, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDR_lit_gen(bool opc_0, Imm<19> imm19, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool LDRSW_lit(Imm<19> imm19, Reg Rt) override {
        ChooseScratch(Reg::R18, Rt);
        return Rt == Reg::R18;
    }

    bool STNP_LDNP_gen(Imm<1> upper_opc, Imm<1> L, Imm<7> imm7, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STNP_LDNP_fpsimd(Imm<2> opc, Imm<1> L, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STP_LDP_gen(Imm<2> opc, bool not_postindex, bool wback, Imm<1> L, Imm<7> imm7, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STP_LDP_fpsimd(Imm<2> opc, bool not_postindex, bool wback, Imm<1> L, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STGP_1(Imm<7> offset_imm, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STGP_2(Imm<7> offset_imm, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STGP_3(Imm<7> offset_imm, Reg Rt2, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STRx_LDRx_imm_1(Imm<2> size, Imm<2> opc, Imm<9> imm9, bool not_postindex, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STRx_LDRx_imm_2(Imm<2> size, Imm<2> opc, Imm<12> imm12, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STURx_LDURx(Imm<2> size, Imm<2> opc, Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool PRFM_imm(Imm<12> imm12, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool PRFM_unscaled_imm(Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STR_imm_fpsimd_1(Imm<2> size, Imm<1> opc_1, Imm<9> imm9, bool not_postindex, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STR_imm_fpsimd_2(Imm<2> size, Imm<1> opc_1, Imm<12> imm12, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LDR_imm_fpsimd_1(Imm<2> size, Imm<1> opc_1, Imm<9> imm9, bool not_postindex, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LDR_imm_fpsimd_2(Imm<2> size, Imm<1> opc_1, Imm<12> imm12, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STUR_fpsimd(Imm<2> size, Imm<1> opc_1, Imm<9> imm9, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LDUR_fpsimd(Imm<2> size, Imm<1> opc_1, Imm<9> imm9, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STTRB(Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDTRB(Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDTRSB(Imm<2> opc, Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STTRH(Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDTRH(Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDTRSH(Imm<2> opc, Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STTR(Imm<2> size, Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDTR(Imm<2> size, Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDTRSW(Imm<9> imm9, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDADDB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDCLRB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDEORB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSETB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSMAXB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSMINB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDUMAXB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDUMINB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool SWPB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDAPRB(Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDADDH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDCLRH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDEORH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSETH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSMAXH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSMINH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDUMAXH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDUMINH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool SWPH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDAPRH(Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDADD(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDCLR(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDEOR(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSET(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSMAX(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDSMIN(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDUMAX(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDUMIN(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool SWP(bool A, bool R, Reg Rs, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rs);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rs == Reg::R18;
    }

    bool LDAPR(Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STRx_reg(Imm<2> size, Imm<1> opc_1, Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rm);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rm == Reg::R18;
    }

    bool LDRx_reg(Imm<2> size, Imm<1> opc_1, Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt, Rm);
        return Rn == Reg::R18 || Rt == Reg::R18 || Rm == Reg::R18;
    }

    bool STR_reg_fpsimd(Imm<2> size, Imm<1> opc_1, Reg Rm, Imm<3> option, bool S, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool LDR_reg_fpsimd(Imm<2> size, Imm<1> opc_1, Reg Rm, Imm<3> option, bool S, Reg Rn, Vec Vt) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool STG_1(Imm<9> imm9, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STG_2(Imm<9> imm9, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STG_3(Imm<9> imm9, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LDG(Imm<9> offset_imm, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STZG_1(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STZG_2(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STZG_3(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool ST2G_1(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool ST2G_2(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool ST2G_3(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STGV(Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool STZ2G_1(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STZ2G_2(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool STZ2G_3(Imm<9> offset_imm, Reg Rn) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool LDGV(Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool LDRA(bool M, bool S, Imm<9> imm9, bool W, Reg Rn, Reg Rt) override {
        ChooseScratch(Reg::R18, Rn, Rt);
        return Rn == Reg::R18 || Rt == Reg::R18;
    }

    bool UDIV(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SDIV(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool LSLV(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool LSRV(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ASRV(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool RORV(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool CRC32(bool sf, Reg Rm, Imm<2> sz, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool CRC32C(bool sf, Reg Rm, Imm<2> sz, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool PACGA(Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SUBP(Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool IRG(Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool GMI(Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SUBPS(Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool RBIT_int(bool sf, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool REV16_int(bool sf, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool REV(bool sf, bool opc_0, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool CLZ_int(bool sf, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool CLS_int(bool sf, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool REV32_int(Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool PACDA(bool Z, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool PACDB(bool Z, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool AUTDA(bool Z, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool AUTDB(bool Z, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd);
        return Rn == Reg::R18 || Rd == Reg::R18;
    }

    bool AND_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool BIC_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ORR_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ORN_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool EOR_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool EON(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ANDS_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool BICS(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ADD_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ADDS_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SUB_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SUBS_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ADD_ext(bool sf, Reg Rm, Imm<3> option, Imm<3> imm3, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ADDS_ext(bool sf, Reg Rm, Imm<3> option, Imm<3> imm3, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SUB_ext(bool sf, Reg Rm, Imm<3> option, Imm<3> imm3, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SUBS_ext(bool sf, Reg Rm, Imm<3> option, Imm<3> imm3, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ADC(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool ADCS(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SBC(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SBCS(bool sf, Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool CCMN_reg(bool sf, Reg Rm, Cond cond, Reg Rn, Imm<4> nzcv) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool CCMP_reg(bool sf, Reg Rm, Cond cond, Reg Rn, Imm<4> nzcv) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool CCMN_imm(bool sf, Imm<5> imm5, Cond cond, Reg Rn, Imm<4> nzcv) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool CCMP_imm(bool sf, Imm<5> imm5, Cond cond, Reg Rn, Imm<4> nzcv) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool CSEL(bool sf, Reg Rm, Cond cond, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool CSINC(bool sf, Reg Rm, Cond cond, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool CSINV(bool sf, Reg Rm, Cond cond, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool CSNEG(bool sf, Reg Rm, Cond cond, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool MADD(bool sf, Reg Rm, Reg Ra, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm, Ra);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18 || Ra == Reg::R18;
    }

    bool MSUB(bool sf, Reg Rm, Reg Ra, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm, Ra);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18 || Ra == Reg::R18;
    }

    bool SMADDL(Reg Rm, Reg Ra, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm, Ra);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18 || Ra == Reg::R18;
    }

    bool SMSUBL(Reg Rm, Reg Ra, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm, Ra);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18 || Ra == Reg::R18;
    }

    bool SMULH(Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool UMADDL(Reg Rm, Reg Ra, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm, Ra);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18 || Ra == Reg::R18;
    }

    bool UMSUBL(Reg Rm, Reg Ra, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm, Ra);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18 || Ra == Reg::R18;
    }

    bool UMULH(Reg Rm, Reg Rn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rn, Rd, Rm);
        return Rn == Reg::R18 || Rd == Reg::R18 || Rm == Reg::R18;
    }

    bool SQDMLAL_vec_1(Imm<2> size, Reg Rm, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool SQDMLSL_vec_1(Imm<2> size, Reg Rm, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool SQDMULL_vec_1(Imm<2> size, Reg Rm, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn, Rm);
        return Rn == Reg::R18 || Rm == Reg::R18;
    }

    bool DUP_gen(bool Q, Imm<5> imm5, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool SMOV(bool Q, Imm<5> imm5, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool UMOV(bool Q, Imm<5> imm5, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool INS_gen(Imm<5> imm5, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool SCVTF_float_fix(bool sf, Imm<2> type, Imm<6> scale, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool UCVTF_float_fix(bool sf, Imm<2> type, Imm<6> scale, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool FCVTZS_float_fix(bool sf, Imm<2> type, Imm<6> scale, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTZU_float_fix(bool sf, Imm<2> type, Imm<6> scale, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTNS_float(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTNU_float(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool SCVTF_float_int(bool sf, Imm<2> type, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool UCVTF_float_int(bool sf, Imm<2> type, Reg Rn, Vec Vd) override {
        ChooseScratch(Reg::R18, Rn);
        return Rn == Reg::R18;
    }

    bool FCVTAS_float(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTAU_float(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTPS_float(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTPU_float(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTMS_float(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTMU_float(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTZS_float_int(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FCVTZU_float_int(bool sf, Imm<2> type, Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }

    bool FJCVTZS(Vec Vn, Reg Rd) override {
        ChooseScratch(Reg::R18, Rd);
        return Rd == Reg::R18;
    }
};

} // namespace Core