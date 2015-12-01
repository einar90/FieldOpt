/******************************************************************************
 *
 *
 *
 * Created: 30.11.2015 2015 by einar
 *
 * This file is part of the FieldOpt project.
 *
 * Copyright (C) 2015-2015 Einar J.M. Baumann <einar.baumann@ntnu.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *****************************************************************************/

#include "test_fixture_case.h"

namespace {


class CaseTest : public CaseTestFixture {
protected:
    CaseTest(){}
    virtual ~CaseTest(){}
};

TEST_F(CaseTest, Constructor) {
    EXPECT_NO_THROW();
}

TEST_F(CaseTest, UUIDs) {
    EXPECT_FALSE(cases_[0]->id().isNull());
    EXPECT_FALSE(cases_[1]->id().isNull());
    EXPECT_FALSE(cases_[2]->id().isNull());
    EXPECT_FALSE(cases_[3]->id().isNull());
}

TEST_F(CaseTest, ObjectiveValues) {
    EXPECT_THROW(cases_[0]->objective_function_value(), Optimization::ObjectiveFunctionException);
    EXPECT_FLOAT_EQ(100.0, cases_[1]->objective_function_value());
    EXPECT_FLOAT_EQ(-50.0, cases_[2]->objective_function_value());
    EXPECT_FLOAT_EQ(-50.0, cases_[3]->objective_function_value());
}

TEST_F(CaseTest, Equals) {
    EXPECT_FALSE(cases_[0]->Equals(cases_[1]));
    EXPECT_TRUE(cases_[2]->Equals(cases_[3]));
}

TEST_F(CaseTest, NumberOfVariables) {
    EXPECT_EQ(0, cases_[0]->binary_variables().size());
    EXPECT_EQ(3, cases_[0]->integer_variables().size());
    EXPECT_EQ(0, cases_[0]->real_variables().size());

    EXPECT_EQ(0, cases_[1]->binary_variables().size());
    EXPECT_EQ(0, cases_[1]->integer_variables().size());
    EXPECT_EQ(3, cases_[1]->real_variables().size());

    EXPECT_EQ(4, cases_[2]->binary_variables().size());
    EXPECT_EQ(3, cases_[2]->integer_variables().size());
    EXPECT_EQ(3, cases_[2]->real_variables().size());

    EXPECT_EQ(4, cases_[3]->binary_variables().size());
    EXPECT_EQ(3, cases_[3]->integer_variables().size());
    EXPECT_EQ(3, cases_[3]->real_variables().size());
}

TEST_F(CaseTest, VariableValues) {
    EXPECT_EQ(1, cases_[0]->integer_variables()[0]);
    EXPECT_EQ(2, cases_[0]->integer_variables()[1]);
    EXPECT_EQ(5, cases_[0]->integer_variables()[2]);

    EXPECT_FLOAT_EQ(1.0, cases_[1]->real_variables()[0]);
    EXPECT_FLOAT_EQ(4.0, cases_[1]->real_variables()[1]);
    EXPECT_FLOAT_EQ(2.5, cases_[1]->real_variables()[2]);

    EXPECT_EQ(1, cases_[2]->integer_variables()[0]);
    EXPECT_EQ(2, cases_[2]->integer_variables()[1]);
    EXPECT_EQ(5, cases_[2]->integer_variables()[2]);
    EXPECT_FLOAT_EQ(1.0, cases_[2]->real_variables()[0]);
    EXPECT_FLOAT_EQ(4.0, cases_[2]->real_variables()[1]);
    EXPECT_FLOAT_EQ(2.5, cases_[2]->real_variables()[2]);
    EXPECT_TRUE(cases_[2]->binary_variables()[0]);
    EXPECT_TRUE(cases_[2]->binary_variables()[1]);
    EXPECT_FALSE(cases_[2]->binary_variables()[2]);
    EXPECT_FALSE(cases_[2]->binary_variables()[3]);
}

}

