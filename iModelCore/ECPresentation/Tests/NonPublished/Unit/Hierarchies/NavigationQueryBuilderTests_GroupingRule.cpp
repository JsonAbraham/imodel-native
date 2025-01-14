/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavigationQueryBuilderTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_RuleClassFilterIsPolymorphic, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_RuleClassFilterIsPolymorphic)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classB->GetFullName(), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", false);
        NavigationQueryContractPtr contract = MultiECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass), true);
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll()
            .From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(selectClass))
            .GroupByContract(*contract)
            .OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().insert(classB);
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_IgnoresGroupingRulesWithInvalidSchemas, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_IgnoresGroupingRulesWithInvalidSchemas)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, "InvalidSchemaName", "InvalidClassName", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexNavigationQueryPtr grouped = &RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass)), "this").From(selectClass)
            )->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_IgnoresGroupingRulesWithInvalidClasses, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_IgnoresGroupingRulesWithInvalidClasses)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), "InvalidClassName", "", "", "");
    groupingRule->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexNavigationQueryPtr grouped = &RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass)), "this").From(selectClass)
            )->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_WhenAllInstancesAreGrouped, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_SameLabelInstanceGroup_WhenAllInstancesAreGrouped)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    GroupingRuleP groupingRuleA = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRuleA->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRuleA);

    GroupingRuleP groupingRuleB = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    groupingRuleB->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRuleB);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "",
        RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        NavigationQueryContractPtr contractA = MultiECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA), true);
        ComplexNavigationQueryPtr groupedA = ComplexNavigationQuery::Create();
        groupedA->SelectAll();
        groupedA->From(ComplexNavigationQuery::Create()->SelectContract(*contractA, "this").From(selectClassA));
        groupedA->GroupByContract(*contractA);

        SelectClass<ECClass> selectClassB(*classB, "this", true);
        NavigationQueryContractPtr contractB = MultiECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB), true);
        ComplexNavigationQueryPtr groupedB = ComplexNavigationQuery::Create();
        groupedB->SelectAll();
        groupedB->From(ComplexNavigationQuery::Create()->SelectContract(*contractB, "this").From(selectClassB));
        groupedB->GroupByContract(*contractB);

        UnionNavigationQueryPtr sorted = UnionNavigationQuery::Create({ groupedA, groupedB });
        sorted->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().insert(classA);
        sorted->GetResultParametersR().GetSelectInstanceClasses().insert(classB);
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_SameLabelInstanceGroup_WhenSomeInstancesAreGroupedAndSomeAreNot, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECEntityClass typeName="C" />
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_SameLabelInstanceGroup_WhenSomeInstancesAreGroupedAndSomeAreNot)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");

    GroupingRuleP groupingRuleA = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRuleA->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRuleA);

    GroupingRuleP groupingRuleB = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    groupingRuleB->AddGroup(*new SameLabelInstanceGroup());
    m_ruleset->AddPresentationRule(*groupingRuleB);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "",
        RulesEngineTestHelpers::CreateClassNamesList({ classA, classB, classC }), true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClassA(*classA, "this", true);
        NavigationQueryContractPtr contractA = MultiECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClassA), true);
        ComplexNavigationQueryPtr groupedA = ComplexNavigationQuery::Create();
        groupedA->SelectAll();
        groupedA->From(ComplexNavigationQuery::Create()->SelectContract(*contractA, "this").From(selectClassA));
        groupedA->GroupByContract(*contractA);

        SelectClass<ECClass> selectClassB(*classB, "this", true);
        NavigationQueryContractPtr contractB = MultiECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClassB), true);
        ComplexNavigationQueryPtr groupedB = ComplexNavigationQuery::Create();
        groupedB->SelectAll();
        groupedB->From(ComplexNavigationQuery::Create()->SelectContract(*contractB, "this").From(selectClassB));
        groupedB->GroupByContract(*contractB);

        ComplexNavigationQueryPtr notGrouped = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classC,
            *RulesEngineTestHelpers::CreateECInstanceNodesQueryForClass(GetSchemaHelper(), SelectClass<ECClass>(*classC, "this", true)));

        UnionNavigationQueryPtr sorted = UnionNavigationQuery::Create({ groupedA, groupedB, notGrouped });
        sorted->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().insert(classA);
        sorted->GetResultParametersR().GetSelectInstanceClasses().insert(classB);
        sorted->GetResultParametersR().GetSelectInstanceClasses().insert(classC);
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByDirectClass, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_ClassGroup_GroupsByDirectClass)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr, classA->GetId(), true);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(*classA, true, "this"));
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectAll();
        query->From(*grouped);
        query->OrderBy(GetECClassGroupingNodesOrderByClause().c_str());
        query->GetResultParametersR().GetSelectInstanceClasses().clear();
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByBaseClassWhenSelectingFromDerivedClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, Grouping_ClassGroup_GroupsByBaseClassWhenSelectingFromDerivedClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECClassCP classD = GetECClass("D");

    GroupingRuleP groupingRuleB = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    groupingRuleB->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*groupingRuleB);

    GroupingRuleP groupingRuleC = new GroupingRule("", 1, false, classC->GetSchema().GetName(), classC->GetName(), "", "", "");
    groupingRuleC->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*groupingRuleC);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "",
        RulesEngineTestHelpers::CreateClassNamesList({ classB, classC, classD }), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    auto classGroupingQuery = queries[0];
    ValidateQuery(spec, classGroupingQuery, [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr, classA->GetId(), true);

        ComplexNavigationQueryPtr nestedB = ComplexNavigationQuery::Create();
        nestedB->SelectContract(*contract, "this");
        nestedB->From(*classB, false, "this");

        ComplexNavigationQueryPtr nestedC = ComplexNavigationQuery::Create();
        nestedC->SelectContract(*contract, "this");
        nestedC->From(*classC, false, "this");

        ComplexNavigationQueryPtr groupedBC = ComplexNavigationQuery::Create();
        groupedBC->SelectAll();
        groupedBC->From(*UnionNavigationQuery::Create({ nestedB, nestedC }));
        groupedBC->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*groupedBC);
        sorted->OrderBy(GetECClassGroupingNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().clear();
        return sorted;
        });

    auto instancesQuery = queries[1];
    ValidateQuery(spec, instancesQuery, [&]()
        {
        SelectClass<ECClass> selectClass(*classD, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classD, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classD,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(selectClass));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsDerivedClassInstancesByBaseClassWhenSelectingFromBaseClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_ClassGroup_GroupsDerivedClassInstancesByBaseClassWhenSelectingFromBaseClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");

    GroupingRuleP groupingRuleB = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    groupingRuleB->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*groupingRuleB);

    GroupingRuleP groupingRuleC = new GroupingRule("", 1, false, classC->GetSchema().GetName(), classC->GetName(), "", "", "");
    groupingRuleC->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*groupingRuleC);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "",
        RulesEngineTestHelpers::CreateClassNamesList({ classA }), true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    auto classGroupingQuery = queries[0];
    ValidateQuery(spec, classGroupingQuery, [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr, classA->GetId(), true);

        SelectClassWithExcludes<ECClass> selectClassB(*classB, "this", true);
        selectClassB.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classC, ""));
        ComplexNavigationQueryPtr nestedB = ComplexNavigationQuery::Create();
        nestedB->SelectContract(*contract, "this");
        nestedB->From(selectClassB);
        
        ComplexNavigationQueryPtr nestedC = ComplexNavigationQuery::Create();
        nestedC->SelectContract(*contract, "this");
        nestedC->From(*classC, true, "this");

        ComplexNavigationQueryPtr groupedBC = ComplexNavigationQuery::Create();
        groupedBC->SelectAll();
        groupedBC->From(*UnionNavigationQuery::Create({ nestedB, nestedC }));
        groupedBC->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*groupedBC);
        sorted->OrderBy(GetECClassGroupingNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().clear();
        return sorted;
        });

    auto instancesQuery = queries[1];
    ValidateQuery(spec, instancesQuery, [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classA, "this", true);
        selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classB, ""));
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*contract, "this").From(selectClass));
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_GroupsByBaseAndDirectClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_ClassGroup_GroupsByBaseAndDirectClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");

    GroupingRuleP groupingRuleB = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    groupingRuleB->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*groupingRuleB);

    GroupingRuleP groupingRuleC = new GroupingRule("", 1, false, classC->GetSchema().GetName(), classC->GetName(), "", "", "");
    groupingRuleC->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*groupingRuleC);

    AllInstanceNodesSpecification spec(1, false, false, false, true, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr baseClassGroupingContract = ECClassGroupingNodesQueryContract::Create("", nullptr, classA->GetId(), true);
        NavigationQueryContractPtr classGroupingContract = ECClassGroupingNodesQueryContract::Create("", nullptr);

        SelectClassWithExcludes<ECClass> selectClassB(*classB, "this", true);
        selectClassB.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classC, ""));
        ComplexNavigationQueryPtr classBQuery = ComplexNavigationQuery::Create();
        classBQuery->SelectContract(*baseClassGroupingContract, "this");
        classBQuery->From(selectClassB);

        ComplexNavigationQueryPtr classCQuery = ComplexNavigationQuery::Create();
        classCQuery->SelectContract(*baseClassGroupingContract, "this");
        classCQuery->From(*classC, true, "this");

        ComplexNavigationQueryPtr baseClassGroupingQueryGrouped = ComplexNavigationQuery::Create();
        baseClassGroupingQueryGrouped->SelectAll();
        baseClassGroupingQueryGrouped->From(*UnionNavigationQuery::Create({ classBQuery, classCQuery }));
        baseClassGroupingQueryGrouped->GroupByContract(*baseClassGroupingContract);

        SelectClassWithExcludes<ECClass> selectClassA(*classA, "this", true);
        selectClassA.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classB, ""));
        ComplexNavigationQueryPtr classGroupingQueryGrouped = ComplexNavigationQuery::Create();
        classGroupingQueryGrouped->SelectAll();
        classGroupingQueryGrouped->From(
            ComplexNavigationQuery::Create()->SelectContract(*classGroupingContract, "this").From(selectClassA));
        classGroupingQueryGrouped->GroupByContract(*classGroupingContract);

        auto query = UnionNavigationQuery::Create(
            {
            &ComplexNavigationQuery::Create()->SelectAll().From(*baseClassGroupingQueryGrouped),
            &ComplexNavigationQuery::Create()->SelectAll().From(*classGroupingQueryGrouped),
            });
        query->OrderBy(GetECClassGroupingNodesOrderByClause().c_str());
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_ClassGroup_ReturnsChildrenOfClassGroupingNode, R"*(
    <ECEntityClass typeName="A" />
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_ClassGroup_ReturnsChildrenOfClassGroupingNode)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());
    spec.SetDoNotSort(true);

    auto parentNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, true, "MyLabel", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *parentNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *parentNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", true);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));
        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(selectClass);
        return RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *query);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByDirectProperty_WhenSelectingFromGroupedClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ByDirectProperty_WhenSelectingFromGroupedClass)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classA->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, true, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByDirectProperty_WhenSelectingFromMultipleGroupedClasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ByDirectProperty_WhenSelectingFromMultipleGroupedClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    GroupingRuleP groupingRuleA = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    auto groupingSpecA = new PropertyGroup("", "", true, "Prop");
    groupingRuleA->AddGroup(*groupingSpecA);
    m_ruleset->AddPresentationRule(*groupingRuleA);

    GroupingRuleP groupingRuleB = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    auto groupingSpecB = new PropertyGroup("", "", true, "Prop");
    groupingRuleB->AddGroup(*groupingSpecB);
    m_ruleset->AddPresentationRule(*groupingRuleB);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "",
        RulesEngineTestHelpers::CreateClassNamesList({ classA, classB }), true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contractA = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"),
            *classA->GetPropertyP("Prop"), *groupingSpecA, nullptr);
        ComplexNavigationQueryPtr nestedA = ComplexNavigationQuery::Create();
        nestedA->SelectContract(*contractA, "this");
        nestedA->From(*classA, true, "this");
        ComplexNavigationQueryPtr groupedA = &ComplexNavigationQuery::Create()->SelectAll().From(*nestedA).GroupByContract(*contractA);

        NavigationQueryContractPtr contractB = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classB, "this"),
            *classB->GetPropertyP("Prop"), *groupingSpecB, nullptr);
        ComplexNavigationQueryPtr nestedB = ComplexNavigationQuery::Create();
        nestedB->SelectContract(*contractB, "this");
        nestedB->From(*classB, true, "this");
        ComplexNavigationQueryPtr groupedB = &ComplexNavigationQuery::Create()->SelectAll().From(*nestedB).GroupByContract(*contractB);

        UnionNavigationQueryPtr unionQuery = UnionNavigationQuery::Create({ groupedA, groupedB });
        unionQuery->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        unionQuery->GetResultParametersR().GetSelectInstanceClasses().clear();
        return unionQuery;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByDirectPropertyRange_WhenSelectingFromGroupedClass, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ByDirectPropertyRange_WhenSelectingFromGroupedClass)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("", "", "0", "5"));
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("", "", "6", "10"));
    groupingSpec->AddRange(*new PropertyRangeGroupSpecification("", "", "11", "20"));
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classA->GetPropertyP("Prop"), *groupingSpec, nullptr);
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, true, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll().From(*nested).GroupByContract(*contract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_OverridesImageId, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_OverridesImageId)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "TestImageId", true, "Prop");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classA->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, true, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ValueFilteringWithOneValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ValueFilteringWithOneValue)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Prop"));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);

    auto propertyGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECPropertyGroupingNode(nullptr, *classA,
        *classA->GetPropertyP("Prop"), "MyCustomLabel", "", { ECValue(9) }, false, {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        rapidjson::Document groupingValues(rapidjson::kArrayType);
        groupingValues.PushBack(rapidjson::Value(9), groupingValues.GetAllocator());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Where(QueryClauseAndBindings("InVirtualSet(?, [this].[Prop])", { std::make_shared<BoundRapidJsonValueSet>(groupingValues, PRIMITIVETYPE_Integer) }));

        ComplexNavigationQueryPtr grouped = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nested);
        grouped->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ValueFilteringWithMultipleValues, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, Grouping_PropertyGroup_ValueFilteringWithMultipleValues)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    groupingRule->AddGroup(*new PropertyGroup("", "", true, "Prop"));
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);

    auto propertyGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECPropertyGroupingNode(nullptr, *classA,
        *classA->GetPropertyP("Prop"), "MyCustomLabel", "", { ECValue(123), ECValue(456) }, false, {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        rapidjson::Document groupingValues(rapidjson::kArrayType);
        groupingValues.PushBack(rapidjson::Value(123), groupingValues.GetAllocator());
        groupingValues.PushBack(rapidjson::Value(456), groupingValues.GetAllocator());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Where(QueryClauseAndBindings("InVirtualSet(?, [this].[Prop])", { std::make_shared<BoundRapidJsonValueSet>(groupingValues, PRIMITIVETYPE_Integer) }));

        ComplexNavigationQueryPtr grouped = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nested);
        grouped->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ValueFilteringWithOneRange, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ValueFilteringWithOneRange)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroup* groupingSpecification = new PropertyGroup("", "", true, "Prop");
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "1", "5"));
    groupingRule->AddGroup(*groupingSpecification);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);

    auto propertyGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECPropertyGroupingNode(nullptr, *classA,
        *classA->GetPropertyP("Prop"), "MyCustomLabel", "", { ECValue(0) }, true, {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Where("([this].[Prop] BETWEEN ? AND ?)", { std::make_shared<BoundQueryECValue>(ECValue(1)), std::make_shared<BoundQueryECValue>(ECValue(5)) });

        ComplexNavigationQueryPtr grouped = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nested);
        grouped->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ValueFilteringWithOtherRange, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ValueFilteringWithOtherRange)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroup* groupingSpecification = new PropertyGroup("", "", true, "Prop");
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "1", "5"));
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "7", "9"));
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "10", "15"));
    groupingRule->AddGroup(*groupingSpecification);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);

    auto propertyGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECPropertyGroupingNode(nullptr, *classA,
        *classA->GetPropertyP("Prop"), "MyCustomLabel", "", { ECValue(-1) }, true, {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Where("([this].[Prop] NOT BETWEEN ? AND ? AND [this].[Prop] NOT BETWEEN ? AND ? AND [this].[Prop] NOT BETWEEN ? AND ?)",
            { std::make_shared<BoundQueryECValue>(ECValue(1)), std::make_shared<BoundQueryECValue>(ECValue(5)),
            std::make_shared<BoundQueryECValue>(ECValue(7)), std::make_shared<BoundQueryECValue>(ECValue(9)),
            std::make_shared<BoundQueryECValue>(ECValue(10)), std::make_shared<BoundQueryECValue>(ECValue(15)) });

        ComplexNavigationQueryPtr grouped = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nested);
        grouped->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ValueFilteringWithMultipleRanges, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, Grouping_PropertyGroup_ValueFilteringWithMultipleRanges)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    m_ruleset->AddPresentationRule(*groupingRule);

    PropertyGroup* groupingSpecification = new PropertyGroup("", "", true, "Prop");
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "1", "5"));
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "7", "9"));
    groupingSpecification->AddRange(*new PropertyRangeGroupSpecification("", "", "10", "15"));
    groupingRule->AddGroup(*groupingSpecification);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);

    auto propertyGroupingNode = TestNodesFactory(GetConnection(), spec.GetHash(), "").CreateECPropertyGroupingNode(nullptr, *classA,
        *classA->GetPropertyP("Prop"), "MyCustomLabel", "", { ECValue(-1), ECValue(0), ECValue(2) }, true, {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass));

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Where(QueryClauseAndBindings(Utf8String()
            .append("([this].[Prop] NOT BETWEEN ? AND ? AND [this].[Prop] NOT BETWEEN ? AND ? AND [this].[Prop] NOT BETWEEN ? AND ?)")
            .append(" OR ([this].[Prop] BETWEEN ? AND ?)")
            .append(" OR ([this].[Prop] BETWEEN ? AND ?)"),
            {
                std::make_shared<BoundQueryECValue>(ECValue(1)), std::make_shared<BoundQueryECValue>(ECValue(5)), std::make_shared<BoundQueryECValue>(ECValue(7)), std::make_shared<BoundQueryECValue>(ECValue(9)), std::make_shared<BoundQueryECValue>(ECValue(10)), std::make_shared<BoundQueryECValue>(ECValue(15)),
                std::make_shared<BoundQueryECValue>(ECValue(1)), std::make_shared<BoundQueryECValue>(ECValue(5)),
                std::make_shared<BoundQueryECValue>(ECValue(10)), std::make_shared<BoundQueryECValue>(ECValue(15))
            }
        ));

        ComplexNavigationQueryPtr grouped = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nested);
        grouped->OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByDirectProperty_WhenPropertyTypeIsNavigation, R"*(
    <ECEntityClass typeName="A">
        <ECNavigationProperty propertyName="NavigationProp" relationshipName="A_B" direction="Forward" />
    </ECEntityClass>
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" strengthDirection="Backward" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ByDirectProperty_WhenPropertyTypeIsNavigation)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "", true, "NavigationProp");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false,
        "", classA->GetFullName(), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        RelatedClass navRelationship(*classA, SelectClass<ECRelationshipClass>(*relAB, NAVIGATION_QUERY_BUILDER_NAV_CLASS_ALIAS(*relAB)), true, SelectClass<ECClass>(*classB, "parentInstance", true));

        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, selectClass,
            *classA->GetPropertyP("NavigationProp"), *groupingSpec, &navRelationship.GetTargetClass());
        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Join(navRelationship);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll().From(*nested).GroupByContract(*contract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByDirectProperty_WhenGroupingSubclassInstancesAndSelectingFromBaseClass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ByDirectProperty_WhenGroupingSubclassInstancesAndSelectingFromBaseClass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), true);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    NavigationQueryPtr propertyGroupingNodesQuery = queries[0];
    ValidateQuery(spec, propertyGroupingNodesQuery, [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", true);
        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, selectClass,
            *classB->GetPropertyP("Prop"), *groupingSpec, nullptr);
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(ComplexNavigationQuery::Create()->SelectContract(*propertyGroupingContract, "this").From(selectClass));
        grouped->GroupByContract(*propertyGroupingContract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        return grouped;
        });

    NavigationQueryPtr instanceNodesQuery = queries[1];
    ValidateQuery(spec, instanceNodesQuery, [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classA, "this", true);
        selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classB, ""));
        ComplexNavigationQueryPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass)), "this").From(selectClass));
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByDirectProperty_WhenGroupingBaseClassInstancesAndSelectingFromSubclasses, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, Grouping_PropertyGroup_ByDirectProperty_WhenGroupingBaseClassInstancesAndSelectingFromSubclasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "",
        RulesEngineTestHelpers::CreateClassNamesList({ classB, classC }), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classA->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr grouped1 = ComplexNavigationQuery::Create();
        grouped1->SelectContract(*propertyGroupingContract, "this");
        grouped1->From(*classB, false, "this");

        ComplexNavigationQueryPtr grouped2 = ComplexNavigationQuery::Create();
        grouped2->SelectContract(*propertyGroupingContract, "this");
        grouped2->From(*classC, false, "this");

        ComplexNavigationQueryPtr grouping = ComplexNavigationQuery::Create();
        grouping->SelectAll();
        grouping->From(*UnionNavigationQuery::Create({ grouped1, grouped2 }));
        grouping->GroupByContract(*propertyGroupingContract);
        grouping->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouping->GetResultParametersR().GetSelectInstanceClasses().clear();
        return grouping;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByRelatedInstanceProperty_WhenGroupingRelatedInstanceClassDirectly, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, Grouping_PropertyGroup_ByRelatedInstanceProperty_WhenGroupingRelatedInstanceClassDirectly)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(), classB->GetFullName(), "b"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classB, "b"), *classB->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*propertyGroupingContract, "this");
        query->From(*classA, false, "this");
        query->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classB, "b", true)));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*propertyGroupingContract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        grouped->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByRelatedInstanceProperty_WhenGroupingByRelatedInstanceSubclass, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ByRelatedInstanceProperty_WhenGroupingByRelatedInstanceSubclass)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classC->GetSchema().GetName(), classC->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relAB->GetFullName(), classB->GetFullName(), "b"));

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(2, queries.size());

    NavigationQueryPtr propertyGroupingNodesQuery = queries[0];
    ValidateQuery(spec, propertyGroupingNodesQuery, [&]()
        {
        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classC, "b"), *classC->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectContract(*propertyGroupingContract, "this");
        grouped->From(*classA, false, "this");
        grouped->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, SelectClass<ECClass>(*classC, "b", true), true));

        ComplexNavigationQueryPtr grouping = ComplexNavigationQuery::Create();
        grouping->SelectAll();
        grouping->From(*grouped);
        grouping->GroupByContract(*propertyGroupingContract);
        grouping->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouping->GetResultParametersR().GetSelectInstanceClasses().clear();
        grouping->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return grouping;
        });

    NavigationQueryPtr instanceNodesQuery = queries[1];
    ValidateQuery(spec, instanceNodesQuery, [&]()
        {
        SelectClassWithExcludes<ECClass> targetClass(*classB, "b", true);
        targetClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classC, ""));
        RelatedClass relatedInstancePath(*classA, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), true, targetClass, true);
        SelectClass<ECClass> selectClass(*classA, "this", false);
        ComplexNavigationQueryPtr sorted = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA,
            ComplexNavigationQuery::Create()->SelectContract(*ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedInstancePath} }), { RelatedClassPath{relatedInstancePath} }), "this")
            .From(selectClass)
            .Join(relatedInstancePath));
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByRelationshipProperty_WhenUsedWithRelatedInstanceNodesSpecification, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B" />
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECRelationshipClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_ByRelationshipProperty_WhenUsedWithRelatedInstanceNodesSpecification)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto rootInstanceNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, relAB->GetSchema().GetName(), relAB->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 0, "", RequiredRelationDirection_Forward,
        "", relAB->GetFullName(), classB->GetFullName());

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *rootInstanceNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr propertyGroupingContract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), *relAB->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*propertyGroupingContract, "this");
        query->From(*classB, true, "this");
        query->Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false));
        query->Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(*propertyGroupingContract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        grouped->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_ByRelationshipProperty_WhenTargetingMultipleClasses, R"*(
    <ECEntityClass typeName="A" />
    <ECEntityClass typeName="B">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="D">
        <BaseClass>B</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="A_B" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ab" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="ba" polymorphic="true">
            <Class class="B" />
        </Target>
        <ECProperty propertyName="Prop" typeName="int" />
    </ECRelationshipClass>
)*");
TEST_F(NavigationQueryBuilderTests, Grouping_PropertyGroup_ByRelationshipProperty_WhenTargetingMultipleClasses)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classC = GetECClass("C");
    ECClassCP classD = GetECClass("D");
    ECRelationshipClassCP relAB = GetECClass("A_B")->GetRelationshipClassCP();

    auto rootInstanceNode = TestNodesHelper::CreateInstanceNode(GetConnection(), *classA);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *rootInstanceNode);

    // create the rules
    GroupingRuleP groupingRule = new GroupingRule("", 1, false, relAB->GetSchema().GetName(), relAB->GetName(), "", "", "");
    auto groupingSpec = new PropertyGroup("", "", true, "Prop", "");
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    RelatedInstanceNodesSpecification spec(1, false, false, false, false, false, false, false, 0,
        "", RequiredRelationDirection_Forward, "", relAB->GetFullName(), RulesEngineTestHelpers::CreateClassNamesList({ classC, classD }));

    // request for root nodes
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, spec, *rootInstanceNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contractC = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), *relAB->GetPropertyP("Prop"), *groupingSpec, nullptr);
        ComplexNavigationQueryPtr queryC = ComplexNavigationQuery::Create();
        queryC->SelectContract(*contractC, "this");
        queryC->From(*classC, true, "this");
        queryC->Join(RelatedClass(*classC, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 0)), false, SelectClass<ECClass>(*classA, "related", true), false));
        queryC->Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        NavigationQueryContractPtr contractD = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 1)), *relAB->GetPropertyP("Prop"), *groupingSpec, nullptr);
        ComplexNavigationQueryPtr queryD = ComplexNavigationQuery::Create();
        queryD->SelectContract(*contractD, "this");
        queryD->From(*classD, true, "this");
        queryD->Join(RelatedClass(*classD, SelectClass<ECRelationshipClass>(*relAB, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAB, 1)), false, SelectClass<ECClass>(*classA, "related", true), false));
        queryD->Where("[related].[ECInstanceId] IN (?)", { std::make_shared<BoundQueryId>(ECInstanceId((uint64_t)123)) });

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*UnionNavigationQuery::Create({ queryC, queryD }));
        grouped->GroupByContract(*ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*relAB, ""), *relAB->GetPropertyP("Prop"), *groupingSpec, nullptr));
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(ECRelatedInstanceDirection::Forward);
        grouped->GetResultParametersR().GetUsedRelationshipClasses().insert(relAB);
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsByRawPropertyValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsByRawPropertyValue)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classA->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, false, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().clear();
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_SortsByRawPropertyValueWhileGroupingByDisplayLabel, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_SortsByRawPropertyValueWhileGroupingByDisplayLabel)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::DisplayLabel);
    groupingSpec->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classA->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, false, "this");

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->GroupByContract(*contract);
        sorted->OrderBy(Utf8PrintfString("[%s]", ECPropertyGroupingNodesQueryContract::GroupingValueFieldName).c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().clear();
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PropertyGroup_GroupsAndSortsByRawPropertyValue, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F (NavigationQueryBuilderTests, Grouping_PropertyGroup_GroupsAndSortsByRawPropertyValue)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
    PropertyGroupP groupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingSpec->SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    groupingSpec->SetSortingValue(PropertyGroupingValue::PropertyValue);
    groupingRule->AddGroup(*groupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    InstanceNodesOfSpecificClassesSpecification spec(1, false, false, false, false, false, false, "", classA->GetFullName(), false);
    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classA->GetPropertyP("Prop"), *groupingSpec, nullptr);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, false, "this");

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*nested);
        sorted->GroupByContract(*contract);
        sorted->OrderBy(Utf8String("[").append(ECPropertyGroupingNodesQueryContract::GroupingValueFieldName).append("]").c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().clear();
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
DEFINE_SCHEMA(Grouping_PicksActiveGroupingSpecificationFromLocalState, R"*(
    <ECEntityClass typeName="A">
        <ECProperty propertyName="Prop" typeName="int" />
    </ECEntityClass>
)*");
TEST_F(NavigationQueryBuilderTests, Grouping_PicksActiveGroupingSpecificationFromLocalState)
    {
    ECClassCP classA = GetECClass("A");

    GroupingRuleP groupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "TestSettingsId");
    auto classGroupingSpec = new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName());
    groupingRule->AddGroup(*classGroupingSpec);
    auto propertyGroupingSpec = new PropertyGroup("", "", true, "Prop");
    groupingRule->AddGroup(*propertyGroupingSpec);
    m_ruleset->AddPresentationRule(*groupingRule);

    AllInstanceNodesSpecification spec(1, false, false, false, false, false, GetECSchema()->GetName());

    TestLocalState localState;
    GetBuilder().GetParameters().SetLocalState(localState);
    int localStateRequestsCount = 0;
    localState.SetGetHandler([&localStateRequestsCount](Utf8CP ns, Utf8CP key) -> Json::Value
        {
        EXPECT_STREQ(RULES_ENGINE_ACTIVE_GROUPS_LOCAL_STATE_NAMESPACE, ns);
        EXPECT_STREQ("TestSettingsId", key);
        ++localStateRequestsCount;
        return 1;
        });

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, spec);
    ASSERT_EQ(1, queries.size());
    EXPECT_EQ(1, localStateRequestsCount);

    ValidateQuery(spec, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classA->GetPropertyP("Prop"), *propertyGroupingSpec, nullptr);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, true, "this");

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        return grouped;
        });
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilderMultiLevelGroupingTests : NavigationQueryBuilderTests
    {
    DEFINE_T_SUPER(NavigationQueryBuilderTests);

    std::unique_ptr<InstanceNodesOfSpecificClassesSpecification> m_specification;
    PropertyGroupP m_propertyGroupingSpecPropA;
    PropertyGroupP m_propertyGroupingSpecPropB;
    PropertyGroupP m_propertyGroupingSpecPropC;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetUp() override
        {
        T_Super::SetUp();

        ECClassCP classA = GetECClass("A");
        ECClassCP classB = GetECClass("B");
        ECClassCP classC = GetECClass("C");
        ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

        GroupingRuleP baseClassGroupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
        baseClassGroupingRule->AddGroup(*new ClassGroup("", true, classA->GetSchema().GetName(), classA->GetName()));
        m_ruleset->AddPresentationRule(*baseClassGroupingRule);

        GroupingRuleP propertyGroupingRule1 = new GroupingRule("", 2, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
        propertyGroupingRule1->AddGroup(*(m_propertyGroupingSpecPropA = new PropertyGroup("", "", true, "PropA")));
        m_ruleset->AddPresentationRule(*propertyGroupingRule1);

        GroupingRuleP propertyGroupingRule2 = new GroupingRule("", 1, false, classB->GetSchema().GetName(), classB->GetName(), "", "", "");
        propertyGroupingRule2->AddGroup(*(m_propertyGroupingSpecPropB = new PropertyGroup("", "", true, "PropB")));
        m_ruleset->AddPresentationRule(*propertyGroupingRule2);

        GroupingRuleP propertyGroupingRule3 = new GroupingRule("", 1, false, classC->GetSchema().GetName(), classC->GetName(), "", "", "");
        propertyGroupingRule3->AddGroup(*(m_propertyGroupingSpecPropC = new PropertyGroup("", "", true, "PropC")));
        m_ruleset->AddPresentationRule(*propertyGroupingRule3);

        GroupingRuleP sameLabelInstanceGroupingRule = new GroupingRule("", 1, false, classA->GetSchema().GetName(), classA->GetName(), "", "", "");
        sameLabelInstanceGroupingRule->AddGroup(*new SameLabelInstanceGroup(""));
        m_ruleset->AddPresentationRule(*sameLabelInstanceGroupingRule);

        m_specification = std::make_unique<InstanceNodesOfSpecificClassesSpecification>(1, false, false, false, true, true, true, "", classA->GetFullName(), true);
        m_specification->AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Forward, relAC->GetFullName(), classC->GetFullName(), "c"));
        }

    ECSchemaCP GetECSchema() {return T_Super::GetECSchema(BeTest::GetNameOfCurrentTestCase());}
    ECClassCP GetECClass(Utf8CP className) {return T_Super::GetECClass(BeTest::GetNameOfCurrentTestCase(), className);}
    };

DEFINE_SCHEMA(NavigationQueryBuilderMultiLevelGroupingTests, R"*(
    <ECEntityClass typeName="A">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="PropA" typeName="long" />
    </ECEntityClass>
    <ECEntityClass typeName="B">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropB" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="C">
        <ECProperty propertyName="PropC" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="A_C" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="ac" polymorphic="true">
            <Class class="A"/>
        </Source>
        <Target multiplicity="(0..1)" roleLabel="ca" polymorphic="true">
            <Class class="C" />
        </Target>
    </ECRelationshipClass>
)*");

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderMultiLevelGroupingTests, RootNodesQueryReturnsBaseClassGroupingNodes)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_rootNodeRule, *m_specification);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(*m_specification, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr, classA->GetId(), true);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classA, true, "this");
        nested->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, "c", true)));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().clear();
        sorted->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return sorted;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderMultiLevelGroupingTests, BaseClassNodeChildrenQueryReturnsDirectClassGroupingNodes)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    auto baseClassGroupingNode = TestNodesFactory(GetConnection(), m_specification->GetHash(), "").CreateECClassGroupingNode(nullptr, *classA, true, "test", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *baseClassGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification, *baseClassGroupingNode);
    ASSERT_EQ(2, queries.size());

    NavigationQueryPtr classGroupingNodesQuery = queries[0];
    ValidateQuery(*m_specification, classGroupingNodesQuery, [&]()
        {
        SelectClassWithExcludes<ECClass> selectClass(*classA, "this", true);
        selectClass.GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*classA, "", false));

        NavigationQueryContractPtr contract = ECClassGroupingNodesQueryContract::Create("", nullptr);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Join(RelatedClass(*classA, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, "c", true)));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectAll();
        sorted->From(*grouped);
        sorted->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().clear();
        sorted->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return sorted;
        });

    NavigationQueryPtr instanceNodesQuery = queries[1];
    ValidateQuery(*m_specification, instanceNodesQuery, [&]()
        {
        SelectClass<ECClass> selectClass(*classA, "this", false);
        RelatedClass relatedInstanceClass(*classA, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, "c", true));

        NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", classA, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedInstanceClass} }), { RelatedClassPath{relatedInstanceClass} });

        ComplexNavigationQueryPtr nestedQuery = ComplexNavigationQuery::Create();
        nestedQuery->SelectContract(*contract, "this");
        nestedQuery->From(selectClass);
        nestedQuery->Join(relatedInstanceClass);

        ComplexNavigationQueryPtr query = RulesEngineTestHelpers::CreateMultiECInstanceNodesQuery(*classA, *nestedQuery);
        query->OrderBy(GetECInstanceNodesOrderByClause().c_str());
        query->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return query;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderMultiLevelGroupingTests, DirectClassNodeChildrenQueryReturnsFirstLevelPropertyGroupingNodes)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    TestNodesFactory nodesFactory(GetConnection(), m_specification->GetHash(), "");

    auto baseClassGroupingNode = nodesFactory.CreateECClassGroupingNode(nullptr, *classA, true, "test", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *baseClassGroupingNode);

    auto classGroupingNode = nodesFactory.CreateECClassGroupingNode(baseClassGroupingNode->GetKey().get(), *classB, false, "test", {});
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification, *classGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(*m_specification, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classA, "this"), *classB->GetPropertyP("PropA"), *m_propertyGroupingSpecPropA, nullptr);

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classB, false, "this");
        nested->Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, "c", true)));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        grouped->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderMultiLevelGroupingTests, FirstLevelPropertyGroupingNodeChildrenQueryReturnsSecondLevelPropertyGroupingNodes)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    TestNodesFactory nodesFactory(GetConnection(), m_specification->GetHash(), "");

    auto baseClassGroupingNode = nodesFactory.CreateECClassGroupingNode(nullptr, *classA, true, "test", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *baseClassGroupingNode);

    auto classGroupingNode = nodesFactory.CreateECClassGroupingNode(baseClassGroupingNode->GetKey().get(), *classB, false, "test", {});
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    auto propertyGroupingNode = nodesFactory.CreateECPropertyGroupingNode(classGroupingNode->GetKey().get(), *classB, *classB->GetPropertyP("PropA"), "test1", "", { ECValue((uint64_t)9) }, false, {});
    propertyGroupingNode->SetParentNode(*classGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification, *propertyGroupingNode);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(*m_specification, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classB, "this"), *classB->GetPropertyP("PropB"), *m_propertyGroupingSpecPropB, nullptr);

        rapidjson::Document groupingValuesA(rapidjson::kArrayType);
        groupingValuesA.PushBack(rapidjson::Value("0x9"), groupingValuesA.GetAllocator());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classB, false, "this");
        nested->Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, "c", true)));
        nested->Where(QueryClauseAndBindings("InVirtualSet(?, [this].[PropA])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesA, PRIMITIVETYPE_Long) }));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        grouped->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderMultiLevelGroupingTests, SecondLevelPropertyGroupingNodeChildrenQueryReturnsThirdPropertyGroupingNodes)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    TestNodesFactory nodesFactory(GetConnection(), m_specification->GetHash(), "");

    auto baseClassGroupingNode = nodesFactory.CreateECClassGroupingNode(nullptr, *classA, true, "test", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *baseClassGroupingNode);

    auto classGroupingNode = nodesFactory.CreateECClassGroupingNode(baseClassGroupingNode->GetKey().get(), *classB, false, "test", {});
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    auto propertyGroupingNode1 = nodesFactory.CreateECPropertyGroupingNode(classGroupingNode->GetKey().get(), *classB, *classB->GetPropertyP("PropA"), "test1", "", { ECValue((uint64_t)9) }, false, {});
    propertyGroupingNode1->SetParentNode(*classGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode1);

    auto propertyGroupingNode2 = nodesFactory.CreateECPropertyGroupingNode(propertyGroupingNode1->GetKey().get(), *classB, *classB->GetPropertyP("PropB"), "test2", "", { ECValue("TestGroupingDescription") }, false, {});
    propertyGroupingNode2->SetParentNode(*propertyGroupingNode1);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode2);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification, *propertyGroupingNode2);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(*m_specification, queries[0], [&]()
        {
        NavigationQueryContractPtr contract = ECPropertyGroupingNodesQueryContract::Create("", nullptr, SelectClass<ECClass>(*classC, "c"), *classC->GetPropertyP("PropC"), *m_propertyGroupingSpecPropC, nullptr);

        rapidjson::Document groupingValuesA(rapidjson::kArrayType);
        groupingValuesA.PushBack(rapidjson::Value("0x9"), groupingValuesA.GetAllocator());

        rapidjson::Document groupingValuesB(rapidjson::kArrayType);
        groupingValuesB.PushBack(rapidjson::Value("TestGroupingDescription"), groupingValuesB.GetAllocator());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(*classB, false, "this");
        nested->Join(RelatedClass(*classB, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, "c", true)));
        nested->Where(QueryClauseAndBindings("InVirtualSet(?, [this].[PropB])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesB, PRIMITIVETYPE_String) }));
        nested->Where(QueryClauseAndBindings("InVirtualSet(?, [this].[PropA])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesA, PRIMITIVETYPE_Long) }));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*nested);
        grouped->GroupByContract(*contract);
        grouped->OrderBy(GetECPropertyGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        grouped->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderMultiLevelGroupingTests, ThirdLevelPropertyGroupingNodeChildrenQueryReturnsLabelGroupingNodes)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    TestNodesFactory nodesFactory(GetConnection(), m_specification->GetHash(), "");

    auto baseClassGroupingNode = nodesFactory.CreateECClassGroupingNode(nullptr, *classA, true, "test", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *baseClassGroupingNode);

    auto classGroupingNode = nodesFactory.CreateECClassGroupingNode(baseClassGroupingNode->GetKey().get(), *classB, false, "test", {});
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    auto propertyGroupingNode1 = nodesFactory.CreateECPropertyGroupingNode(classGroupingNode->GetKey().get(), *classB, *classB->GetPropertyP("PropA"), "test1", "", { ECValue((uint64_t)9) }, false, {});
    propertyGroupingNode1->SetParentNode(*classGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode1);

    auto propertyGroupingNode2 = nodesFactory.CreateECPropertyGroupingNode(propertyGroupingNode1->GetKey().get(), *classB, *classB->GetPropertyP("PropB"), "test2", "", { ECValue("TestGroupingDescription") }, false, {});
    propertyGroupingNode2->SetParentNode(*propertyGroupingNode1);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode2);

    auto propertyGroupingNode3 = nodesFactory.CreateECPropertyGroupingNode(propertyGroupingNode2->GetKey().get(), *classC, *classC->GetPropertyP("PropC"), "test3", "", { ECValue(99) }, false, {});
    propertyGroupingNode3->SetParentNode(*propertyGroupingNode2);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode3);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification, *propertyGroupingNode3);
    ASSERT_EQ(1, queries.size());

    ValidateQuery(*m_specification, queries[0], [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", false);
        RelatedClass relatedInstanceClass(*classB, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, "c", true));
        NavigationQueryContractPtr contract = DisplayLabelGroupingNodesQueryContract::Create("", nullptr, classB, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedInstanceClass} }));

        rapidjson::Document groupingValuesA(rapidjson::kArrayType);
        groupingValuesA.PushBack(rapidjson::Value("0x9"), groupingValuesA.GetAllocator());

        rapidjson::Document groupingValuesB(rapidjson::kArrayType);
        groupingValuesB.PushBack(rapidjson::Value("TestGroupingDescription"), groupingValuesB.GetAllocator());

        rapidjson::Document groupingValuesC(rapidjson::kArrayType);
        groupingValuesC.PushBack(rapidjson::Value(99), groupingValuesC.GetAllocator());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this")
            .From(selectClass)
            .Join(relatedInstanceClass)
            .Where(QueryClauseAndBindings("InVirtualSet(?, [c].[PropC])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesC, PRIMITIVETYPE_Integer) }))
            .Where(QueryClauseAndBindings("InVirtualSet(?, [this].[PropB])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesB, PRIMITIVETYPE_String) }))
            .Where(QueryClauseAndBindings("InVirtualSet(?, [this].[PropA])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesA, PRIMITIVETYPE_Long) }));

        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectContract(*contract, "this")
            .From(*nested)
            .GroupByContract(*contract)
            .OrderBy(GetLabelGroupingNodesOrderByClause().c_str());
        grouped->GetResultParametersR().GetSelectInstanceClasses().clear();
        grouped->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        grouped->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return grouped;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (NavigationQueryBuilderMultiLevelGroupingTests, LabelGroupingNodeChildrenQueryReturnsInstanceNodes)
    {
    ECClassCP classA = GetECClass("A");
    ECClassCP classB = GetECClass("B");
    ECClassCP classC = GetECClass("C");
    ECRelationshipClassCP relAC = GetECClass("A_C")->GetRelationshipClassCP();

    TestNodesFactory nodesFactory(GetConnection(), m_specification->GetHash(), "");

    auto baseClassGroupingNode = nodesFactory.CreateECClassGroupingNode(nullptr, *classA, true, "test", {});
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *baseClassGroupingNode);

    auto classGroupingNode = nodesFactory.CreateECClassGroupingNode(baseClassGroupingNode->GetKey().get(), *classB, false, "test", {});
    classGroupingNode->SetParentNode(*baseClassGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *classGroupingNode);

    auto propertyGroupingNode1 = nodesFactory.CreateECPropertyGroupingNode(classGroupingNode->GetKey().get(), *classB, *classB->GetPropertyP("PropA"), "test1", "", { ECValue((uint64_t)9) }, false, {});
    propertyGroupingNode1->SetParentNode(*classGroupingNode);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode1);

    auto propertyGroupingNode2 = nodesFactory.CreateECPropertyGroupingNode(propertyGroupingNode1->GetKey().get(), *classB, *classB->GetPropertyP("PropB"), "test2", "", { ECValue("TestGroupingDescription") }, false, {});
    propertyGroupingNode2->SetParentNode(*propertyGroupingNode1);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode2);

    auto propertyGroupingNode3 = nodesFactory.CreateECPropertyGroupingNode(propertyGroupingNode2->GetKey().get(), *classC, *classC->GetPropertyP("PropC"), "test3", "", { ECValue(99) }, false, {});
    propertyGroupingNode3->SetParentNode(*propertyGroupingNode2);
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *propertyGroupingNode3);

    NavNodePtr labelGroupingNode = nodesFactory.CreateDisplayLabelGroupingNode(propertyGroupingNode3->GetKey().get(), "test", 1);
    labelGroupingNode->SetParentNode(*propertyGroupingNode3);
    labelGroupingNode->SetInstanceKeysSelectQuery(StringGenericQuery::Create(CHILD_INSTANCE_KEYS_QUERY));
    RulesEngineTestHelpers::CacheNode(m_nodesCache, *labelGroupingNode);

    bvector<NavigationQueryPtr> queries = GetBuilder().GetQueries(*m_childNodeRule, *m_specification, *labelGroupingNode);
    ASSERT_EQ(1, queries.size());

    NavigationQueryPtr query = queries[0];
    ValidateQuery(*m_specification, query, [&]()
        {
        SelectClass<ECClass> selectClass(*classB, "this", false);
        RelatedClass relatedInstanceClass(*classB, SelectClass<ECRelationshipClass>(*relAC, RULES_ENGINE_RELATED_CLASS_ALIAS(*relAC, 0)), true, SelectClass<ECClass>(*classC, "c", true));
        NavigationQueryContractPtr contract = MultiECInstanceNodesQueryContract::Create("", classB, CreateDisplayLabelField(selectClass, { RelatedClassPath{relatedInstanceClass} }), true, { RelatedClassPath{relatedInstanceClass} });

        rapidjson::Document groupingValuesA(rapidjson::kArrayType);
        groupingValuesA.PushBack(rapidjson::Value("0x9"), groupingValuesA.GetAllocator());

        rapidjson::Document groupingValuesB(rapidjson::kArrayType);
        groupingValuesB.PushBack(rapidjson::Value("TestGroupingDescription"), groupingValuesB.GetAllocator());

        rapidjson::Document groupingValuesC(rapidjson::kArrayType);
        groupingValuesC.PushBack(rapidjson::Value(99), groupingValuesC.GetAllocator());

        ComplexNavigationQueryPtr nested = ComplexNavigationQuery::Create();
        nested->SelectContract(*contract, "this");
        nested->From(selectClass);
        nested->Join(relatedInstanceClass);
        SetLabelGroupingNodeChildrenWhereClause(*nested);
        nested->Where(QueryClauseAndBindings("InVirtualSet(?, [c].[PropC])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesC, PRIMITIVETYPE_Integer) }));
        nested->Where(QueryClauseAndBindings("InVirtualSet(?, [this].[PropB])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesB, PRIMITIVETYPE_String) }));
        nested->Where(QueryClauseAndBindings("InVirtualSet(?, [this].[PropA])", { std::make_shared<BoundRapidJsonValueSet>(groupingValuesA, PRIMITIVETYPE_Long) }));

        ComplexNavigationQueryPtr sorted = ComplexNavigationQuery::Create();
        sorted->SelectContract(*contract, "this")
            .From(*nested)
            .GroupByContract(*contract)
            .OrderBy(GetECInstanceNodesOrderByClause().c_str());
        sorted->GetResultParametersR().GetSelectInstanceClasses().insert(classB);
        sorted->GetResultParametersR().GetUsedRelationshipClasses().insert(relAC);
        return sorted;
        });
    }
