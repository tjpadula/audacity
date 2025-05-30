/*
* Audacity: A Digital Audio Editor
*/

#include "testing/environment.h"

#include "au3wrap/au3wrapmodule.h"

#include "projectscene/tests/mocks/projectsceneconfigurationmock.h"
#include "project/tests/mocks/projectconfigurationmock.h"

using namespace ::testing;
using namespace au::projectscene;
using namespace au::project;

static muse::testing::SuiteEnvironment trackedit_se
    = muse::testing::SuiteEnvironment()
      .setDependencyModules({ new au::au3::Au3WrapModule(), })
      .setPostInit([]() {
    std::shared_ptr<NiceMock<ProjectSceneConfigurationMock> > projectSceneConfigurator(new NiceMock<ProjectSceneConfigurationMock>(),
                                                                                       [](ProjectSceneConfigurationMock*) {}); // no delete

    static std::vector<std::pair<std::string /*name*/, std::string /*color*/> > colors = {
        { "blue", "#0000FF" },
        { "red", "#FF0000" },
    };

    ON_CALL(*projectSceneConfigurator, clipColors())
    .WillByDefault(ReturnRef(colors));

    muse::modularity::globalIoc()->unregister<IProjectSceneConfiguration>("utests");
    muse::modularity::globalIoc()->registerExport<IProjectSceneConfiguration>("utests", projectSceneConfigurator);
}).setPreInit([](){
    std::shared_ptr<NiceMock<ProjectConfigurationMock> > projectConfigurator(new NiceMock<ProjectConfigurationMock>(),
                                                                             [](ProjectConfigurationMock*){});

    ON_CALL(*projectConfigurator, temporaryDir())
    .WillByDefault(Return(""));

    muse::modularity::globalIoc()->registerExport<IProjectConfiguration>("utests", projectConfigurator);
}).setDeInit([](){
    std::shared_ptr<IProjectSceneConfiguration> projectSceneConfiguratorPtr
        = muse::modularity::globalIoc()->resolve<IProjectSceneConfiguration>("utests");
    muse::modularity::globalIoc()->unregister<IProjectSceneConfiguration>("utests");

    std::shared_ptr<IProjectConfiguration> projectConfiguratorPtr
        = muse::modularity::globalIoc()->resolve<IProjectConfiguration>("utests");
    muse::modularity::globalIoc()->unregister<IProjectConfiguration>("utests");

    //! HACK
    //! There are still live pointers to the projectSceneConfiguratorPtr
    //! because of this, the projectSceneConfiguratorPtr generates an error stating that it has not been deleted
    //! This is a hack to remove it manually to get rid of this error
    IProjectSceneConfiguration* projectSceneConfigurator = projectSceneConfiguratorPtr.get();
    delete projectSceneConfigurator;

    IProjectConfiguration* projectConfigurator = projectConfiguratorPtr.get();
    delete projectConfigurator;
});
