// Copyright 2017 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package error_prone

import (
	"android/soong/java/config"
)

func init() {
	// These values are set into build/soong/java/config/config.go so that soong doesn't have any
	// references to external/error_prone, which may not always exist.

	// The checks that are fatal to the build.
	config.ErrorProneChecksError = []string{
		// Errorprone default severity ERROR
		"-Xep:AndroidInjectionBeforeSuper:ERROR",
		"-Xep:ArrayFillIncompatibleType:ERROR",
		"-Xep:AsyncCallableReturnsNull:ERROR",
		"-Xep:AsyncFunctionReturnsNull:ERROR",
		"-Xep:AutoValueConstructorOrderChecker:ERROR",
		"-Xep:BundleDeserializationCast:ERROR",
		"-Xep:ChainingConstructorIgnoresParameter:ERROR",
		"-Xep:CheckReturnValue:ERROR",
		"-Xep:CollectionIncompatibleType:ERROR",
		"-Xep:ComparingThisWithNull:ERROR",
		"-Xep:ComparisonOutOfRange:ERROR",
		"-Xep:CompatibleWithAnnotationMisuse:ERROR",
		"-Xep:CompileTimeConstant:ERROR",
		"-Xep:ConstantOverflow:ERROR",
		"-Xep:DaggerProvidesNull:ERROR",
		"-Xep:DeadException:ERROR",
		"-Xep:DeadThread:ERROR",
		"-Xep:DoNotCall:ERROR",
		"-Xep:EqualsNaN:ERROR",
		"-Xep:EqualsWrongThing:ERROR",
		"-Xep:ErroneousThreadPoolConstructorChecker:ERROR",
		"-Xep:ForOverride:ERROR",
		"-Xep:FormatStringAnnotation:ERROR",
		"-Xep:FunctionalInterfaceMethodChanged:ERROR",
		"-Xep:FuturesGetCheckedIllegalExceptionType:ERROR",
		"-Xep:GuiceAssistedInjectScoping:ERROR",
		"-Xep:GuiceAssistedParameters:ERROR",
		"-Xep:GuiceInjectOnFinalField:ERROR",
		"-Xep:Immutable:ERROR",
		"-Xep:IncompatibleArgumentType:ERROR",
		"-Xep:IndexOfChar:ERROR",
		"-Xep:InexactVarargsConditional:ERROR",
		"-Xep:InfiniteRecursion:ERROR",
		"-Xep:InjectMoreThanOneScopeAnnotationOnClass:ERROR",
		"-Xep:InjectOnMemberAndConstructor:ERROR",
		"-Xep:InvalidPatternSyntax:ERROR",
		"-Xep:IsInstanceOfClass:ERROR",
		"-Xep:JUnit3TestNotRun:ERROR",
		"-Xep:JUnit4ClassAnnotationNonStatic:ERROR",
		"-Xep:JUnit4SetUpNotRun:ERROR",
		"-Xep:JUnit4TearDownNotRun:ERROR",
		"-Xep:JUnitAssertSameCheck:ERROR",
		"-Xep:JavaxInjectOnAbstractMethod:ERROR",
		"-Xep:LiteByteStringUtf8:ERROR",
		"-Xep:LoopConditionChecker:ERROR",
		"-Xep:MissingTestCall:ERROR",
		"-Xep:MockitoUsage:ERROR",
		"-Xep:MoreThanOneInjectableConstructor:ERROR",
		"-Xep:MustBeClosedChecker:ERROR",
		"-Xep:NCopiesOfChar:ERROR",
		"-Xep:NonCanonicalStaticImport:ERROR",
		"-Xep:NonFinalCompileTimeConstant:ERROR",
		"-Xep:OptionalEquality:ERROR",
		"-Xep:OverlappingQualifierAndScopeAnnotation:ERROR",
		"-Xep:PackageInfo:ERROR",
		"-Xep:PrivateSecurityContractProtoAccess:ERROR",
		"-Xep:ProtoFieldNullComparison:ERROR",
		"-Xep:ProtoStringFieldReferenceEquality:ERROR",
		"-Xep:ProvidesMethodOutsideOfModule:ERROR",
		"-Xep:RandomCast:ERROR",
		"-Xep:RestrictedApi:ERROR",
		"-Xep:SelfAssignment:ERROR",
		"-Xep:ShouldHaveEvenArgs:ERROR",
		"-Xep:StreamToString:ERROR",
		"-Xep:SubstringOfZero:ERROR",
		"-Xep:SuppressWarningsDeprecated:ERROR",
		"-Xep:ThrowIfUncheckedKnownChecked:ERROR",
		"-Xep:ThrowNull:ERROR",
		"-Xep:TruthSelfEquals:ERROR",
		"-Xep:TypeParameterQualifier:ERROR",
		"-Xep:UnnecessaryTypeArgument:ERROR",
		"-Xep:UnusedAnonymousClass:ERROR",
		"-Xep:VarTypeName:ERROR",
		// Errorprone default severity WARNING
		// Errorprone default severity SUGGESTION
	}

	// The checks that are not fatal to the build.
	config.ErrorProneChecksWarning = []string{
		// Errorprone default severity ERROR
		"-Xep:ComparisonOutOfRange:WARN",
		"-Xep:EqualsHashCode:WARN",
		"-Xep:GuardedBy:WARN",
		"-Xep:ImmutableAnnotationChecker:WARN",
		"-Xep:ImmutableEnumChecker:WARN",
		"-Xep:IsLoggableTagLength:WARN",
		"-Xep:JUnit4TestNotRun:WARN",
		"-Xep:MissingSuperCall:WARN",
		"-Xep:RectIntersectReturnValueIgnored:WARN",
		"-Xep:SelfAssertion:WARN",
		"-Xep:DuplicateBranches:WARN",
	}

	// The checks that are default-disabled
	config.ErrorProneChecksDefaultDisabled = []string{}

	config.ErrorProneChecksOff = []string{
		// We are not interested in Guava recommendations
		// for String.split.
		"-Xep:StringSplitter:OFF",
		// b/117877739.
		"-Xep:TruthAssertExpected:OFF",
		// b/170172949
		"-Xep:MissingOverride:OFF",
		"-Xep:UnnecessaryParentheses:OFF",
		// This triggers on android R classes, disable until we can ignore
		// the generated R classes
		"-Xep:MutablePublicArray:OFF",
		// These checks crash
		"-Xep:RethrowReflectiveOperationExceptionAsLinkageError:OFF",
		"-Xep:InvalidLink:OFF",
		// Commonly triggers for stubbed methods
		"-Xep:DoNotCallSuggester:OFF",
		"-Xep:MissingSummary:OFF",
		// This check increates the `platformprotos` module's build
		// time by ~15 minutes
		"-Xep:SameNameButDifferent:OFF",
		// Noisy and requires projects to add a dependency on errorprone annotations
		"-Xep:CanIgnoreReturnValueSuggester:OFF",
		// Data classes are encouraged to override toString(), but it is not a strict
		// requirement. The warning is overtriggered when source depends on the API stubs, which
		// may not include the toString() method.
		"-Xep:ObjectToString:OFF",
		// Disable the check which is introduced by the Java target 21 until modules 
		// can be fixed individually (b/377918299).
		"-Xep:PatternMatchingInstanceof:OFF",
	}

	config.ErrorProneFlags = []string{
		"-Xdiags:verbose",
		"-XDcompilePolicy=simple",
		"-XDallowBetterNullChecks=false",
		"-XDusePolyAttribution=true",
		"-XDuseStrictMethodClashCheck=true",
		"-XDuseStructuralMostSpecificResolution=true",
		"-XDuseGraphInference=true",
		"-XDandroidCompatible=true",
		// https://github.com/google/error-prone/issues/4595#issuecomment-2424140062
		"--should-stop=ifError=FLOW",
		// As we emit errors as warnings,
		// increase the warning limit.
		"-Xmaxwarns 9999999",

		// Extra flags needed by ErrorProne for OpenJDK9 from
		// http://errorprone.info/docs/installation
		"-J--add-exports=jdk.compiler/com.sun.tools.javac.api=ALL-UNNAMED",
		"-J--add-exports=jdk.compiler/com.sun.tools.javac.file=ALL-UNNAMED",
		"-J--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
		"-J--add-exports=jdk.compiler/com.sun.tools.javac.model=ALL-UNNAMED",
		"-J--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
		"-J--add-exports=jdk.compiler/com.sun.tools.javac.processing=ALL-UNNAMED",
		"-J--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
		"-J--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
		"-J--add-opens=jdk.compiler/com.sun.tools.javac.code=ALL-UNNAMED",
		"-J--add-opens=jdk.compiler/com.sun.tools.javac.comp=ALL-UNNAMED",
	}
}
