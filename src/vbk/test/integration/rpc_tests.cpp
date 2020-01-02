#include <boost/test/unit_test.hpp>

#include <chainparams.h>
#include <consensus/merkle.h>
#include <fakeit.hpp>
#include <rpc/request.h>
#include <rpc/server.h>
#include <test/util/setup_common.h>
#include <univalue.h>
#include <validation.h>
#include <wallet/wallet.h>

#include <vbk/init.hpp>

struct IntegrationTextFixture : public TestChain100Setup {

    IntegrationTextFixture() : TestChain100Setup()
    {
        VeriBlock::InitPopService();
    }
};

BOOST_FIXTURE_TEST_SUITE(rpc_tests, IntegrationTextFixture)

BOOST_AUTO_TEST_CASE(getpopdata_test)
{
    NodeContext node;
    node.chain = interfaces::MakeChain(node);
    node.connman = std::unique_ptr<CConnman>(new CConnman(GetRand(std::numeric_limits<uint64_t>::max()), GetRand(std::numeric_limits<uint64_t>::max())));
    auto& chain = node.chain;
    VeriBlock::InitRpcService(node.connman.get());
    std::shared_ptr<CWallet> wallet = std::make_shared<CWallet>(chain.get(), WalletLocation(), WalletDatabase::CreateDummy());
    AddWallet(wallet);

    int blockHeight = 10;
    CBlockIndex* blockIndex = ChainActive()[blockHeight];
    CBlock block;

    BOOST_CHECK(ReadBlockFromDisk(block, blockIndex, Params().GetConsensus()));

    CDataStream ssBlock(SER_NETWORK, PROTOCOL_VERSION);
    ssBlock << blockIndex->GetBlockHeader();

    uint256 txRoot = BlockMerkleRoot(block);
    auto keystones = VeriBlock::getService<VeriBlock::UtilService>().getKeystoneHashesForTheNextBlock(blockIndex->pprev);
    auto contextInfo = VeriBlock::ContextInfoContainer(blockIndex->nHeight, keystones, txRoot);
    auto authedContext = contextInfo.getAuthenticated();

    JSONRPCRequest request;
    request.strMethod = "getpopdata";
    request.params = UniValue(UniValue::VARR);
    request.fHelp = false;

    request.params.push_back(blockHeight);

    if (RPCIsInWarmup(nullptr)) SetRPCWarmupFinished();

    UniValue result;
    BOOST_CHECK_NO_THROW(result = tableRPC.execute(request));

    BOOST_CHECK(find_value(result.get_obj(), "raw_contextinfocontainer").get_str() == HexStr(authedContext.begin(), authedContext.end()));
    BOOST_CHECK(find_value(result.get_obj(), "block_header").get_str() == HexStr(ssBlock));
    BOOST_CHECK(find_value(result.get_obj(), "last_known_veriblock_blocks").get_array().size() == 15); // number of bootsrap blocks that have been set up in .properties file
    BOOST_CHECK(find_value(result.get_obj(), "last_known_bitcoin_blocks").get_array().size() == 15); // number of bootsrap blocks that have been set up in .properties file
}

BOOST_AUTO_TEST_CASE(submitpop_test)
{   
    JSONRPCRequest request;
    request.strMethod = "submitpop";
    request.params = UniValue(UniValue::VARR);
    request.fHelp = false;
    
    std::vector<uint8_t> atv(100, 1);
    std::vector<uint8_t> vtb(100, 2);
    std::string vtb_str = "02046002011667FF0A897E5D512A0B6DA2F41C479867FE6B3A4CAE2640000013350002A793C872D6F6460E90BED62342BB968195F8C515D3EED7277A09EFAC4BE99F95F0A15628B06BA3B44C0190B5C0495C9B8ACD0701C5235EBBBE9C02011B01000000010CE74F1FB694A001EEBB1D7D08CE6208033F5BF7263EBAD2DE07BBF518672732000000006A47304402200CF4998ABA1682ABEB777E762807A9DD2635A0B77773F66491B83EE3C87099BA022033B7CA24DC520915B8B0200CBDCF95BA6AE866354585AF9C53EE86F27362EBEC012103E5BAF0709C395A82EF0BD63BC8847564AC201D69A8E6BF448D87AA53A1C431AAFFFFFFFF02B7270D00000000001976A9148B9EA8545059F3A922457AFD14DDF3855D8B109988AC0000000000000000536A4C50000013350002A793C872D6F6460E90BED62342BB968195F8C515D3EED7277A09EFAC4BE99F95F0A15628B06BA3B44C0190B5C0495C9B8ACD0701C5235EBBBE9CD4E943EFE1864DF04216615CF92083F40000000002019F040000067B040000000C040000000400000020204D66077FDF24246FFD6B6979DFEDEF5D46588654ADDEB35EDB11E993C131F61220023D1ABE8758C6F917EC0C65674BBD43D66EE14DC667B3117DFC44690C6F5AF120096DDBA03CA952AF133FB06307C24171E53BF50AB76F1EDEABDE5E99F78D4EAD202F32CF1BEE50349D56FC1943AF84F2D2ABDA520F64DC4DB37B2F3DB20B0ECB572093E70120F1B539D0C1495B368061129F30D35F9E436F32D69967AE86031A275620F554378A116E2142F9F6315A38B19BD8A1B2E6DC31201F2D37A058F03C39C06C200824705685CECA003C95140434EE9D8BBBF4474B83FD4ECC2766137DB9A44D7420B7B9E52F3EE8CE4FBB8BE7D6CF66D33A20293F806C69385136662A74453FB162201732C9A35E80D4796BABEA76AACE50B49F6079EA3E349F026B4491CFE720AD17202D9B57E92AB51FE28A587050FD82ABB30ABD699A5CE8B54E7CD49B2A827BCB9920DCBA229ACDC6B7F028BA756FD5ABBFEBD31B4227CD4137D728EC5EA56C457618202CF1439A6DBCC1A35E96574BDDBF2C5DB9174AF5AD0D278FE92E06E4AC349A42500000C020134F09D43659EB53982D9AFB444B96FA4BB58C037D2914000000000000000000CE0B1A9A77DD0DB127B5DF4BC368CD6AC299A9747D991EC2DACBC0B699A2E4A5B3919B5C6C1F2C1773703BC001035000008020FC61CC9D4EAC4B2D14761A4D06AF8A9EF073DCD7FB5E0D000000000000000000A31508D4B101D0AD11E43EF9419C23FC277F67EDAE83C598EE70866DBCEF5E25268B9B5C6C1F2C17E11874AF50000040203F8E3980304439D853C302F6E496285E110E251251531300000000000000000039A72C22268381BD8D9DCFE002F472634A24CF0454DE8B50F89E10891E5FFB1DE08D9B5C6C1F2C1744290A925000000020BAA42E40345A7F826A31D37DB1A5D64B67B72732477422000000000000000000A33AD6BE0634647B26633AB85FA8DE258480BBB25E59C68E48BB0B608B12362B10919B5C6C1F2C1749C4D1F0473045022100F4DCE45EDCC6BFC4A1F44EF04E47E90A348EFD471F742F18B882AC77A8D0E89E0220617CF7C4A22211991687B17126C1BB007A3B2A25C550F75D66B857A8FD9D75E7583056301006072A8648CE3D020106052B8104000A03420004B3C10470C8E8E426F1937758D9FB5E97A1891176CB37D4C12D4AF4107B1AA3E8A8A754C06A22760E44C60642FBA883967C19740D5231336326F7962750C8DF990400000000040000000D202A014E88ED7AB65CDFAA85DAEAB07EEA6CBA5E147F736EDD8D02C2F9DDF0DEC60400000006205B977EA09A554AD56957F662284044E7D37450DDADF7DB3647712F59693997872020D0A3D873EEEEE6A222A75316DCE60B53CA43EAEA09D27F0ECE897303A53AE920C06FE913DCA5DC2736563B80834D69E6DFDF1B1E92383EA62791E410421B6C1120049F68D350EEB8B3DF630C8308B5C8C2BA4CD6210868395B084AF84D19FF0E902000000000000000000000000000000000000000000000000000000000000000002036252DFC621DE420FB083AD9D8767CBA627EDDEEC64E421E9576CEE21297DD0A40000013700002449C60619294546AD825AF03B0935637860679DDD55EE4FD21082E18686EB53C1F4E259E6A0DF23721A0B3B4B7AB5C9B9211070211CAF01C3F010100";
    std::string atv_str = "01580101166772F51AB208D32771AB1506970EEB664462730B838E0203E800010701370100010C6865616465722062797465730112636F6E7465787420696E666F20627974657301117061796F757420696E666F2062797465734630440220398B74708DC8F8AEE68FCE0C47B8959E6FCE6354665DA3ED87A83F708E62AA6B02202E6C00C00487763C55E92C7B8E1DD538B7375D8DF2B2117E75ACBB9DB7DEB3C7583056301006072A8648CE3D020106052B8104000A03420004DE4EE8300C3CD99E913536CF53C4ADD179F048F8FE90E5ADF3ED19668DD1DBF6C2D8E692B1D36EAC7187950620A28838DA60A8C9DD60190C14C59B82CB90319E04000000010400000000201FEC8AA4983D69395010E4D18CD8B943749D5B4F575E88A375DEBDC5ED22531C040000000220000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000040000013880002449C60619294546AD825AF03B0935637860679DDD55EE4FD21082E18686E26BBFDA7D5E4462EF24AE02D67E47D785C9B90F301010000000000010100";

    UniValue vtbs_params(UniValue::VARR);
    vtbs_params.push_back(vtb_str);

    request.params.push_back(atv_str);
    request.params.push_back(vtbs_params);

    if (RPCIsInWarmup(nullptr)) SetRPCWarmupFinished();

    UniValue result;
    BOOST_CHECK_NO_THROW(result = tableRPC.execute(request));

    uint256 popTxHash;
    popTxHash.SetHex(result.get_str());

    BOOST_CHECK(mempool.exists(popTxHash));
}

BOOST_AUTO_TEST_CASE(updatecontext_test)
{
    // This test have to be ran with the clear alt-integration service`s databases
    NodeContext node;
    node.chain = interfaces::MakeChain(node);
    node.connman = std::unique_ptr<CConnman>(new CConnman(GetRand(std::numeric_limits<uint64_t>::max()), GetRand(std::numeric_limits<uint64_t>::max())));
    auto& chain = node.chain;
    VeriBlock::InitRpcService(node.connman.get());
    std::shared_ptr<CWallet> wallet = std::make_shared<CWallet>(chain.get(), WalletLocation(), WalletDatabase::CreateDummy());
    AddWallet(wallet);

    int blockHeight = 10;

    JSONRPCRequest request;
    request.strMethod = "getpopdata";
    request.params = UniValue(UniValue::VARR);
    request.fHelp = false;

    request.params.push_back(blockHeight);

    if (RPCIsInWarmup(nullptr)) SetRPCWarmupFinished();

    UniValue result;
    BOOST_CHECK_NO_THROW(result = tableRPC.execute(request));

    BOOST_CHECK(find_value(result.get_obj(), "last_known_veriblock_blocks").get_array().size() == 15); // number of bootsrap blocks that have been set up in .properties file
    BOOST_CHECK(find_value(result.get_obj(), "last_known_bitcoin_blocks").get_array().size() == 15); // number of bootsrap blocks that have been set up in .properties file


    request.strMethod = "updatecontext";
    request.params = UniValue(UniValue::VARR);
    request.fHelp = false;

    // These values was taken from the alt-integration service default properties file and also was removed from that file
    UniValue veriblock_blocks(UniValue::VARR);
    veriblock_blocks.push_back("00019C9A00026713D7DDB273B344AADD06BF9AFD32E075F6BEC5BADF97549B10936A8468C8CF8CE7253035B52DDE7A1B1CEDD3BC5DE540480405F5E10E87A0F1");

    UniValue bitcoin_blocks(UniValue::VARR);
    bitcoin_blocks.push_back("00000020b15d70ede23ca82d38a9418914d836cf75251b8027b9a9d53c02000000000000d455849c387207d5b3ca9577796874dcdf22e5a31e6bdc8b15a6aaaf44e7de1f7a3ee55dffff001d31f589dc");

    request.params.push_back(bitcoin_blocks);
    request.params.push_back(veriblock_blocks);

    if (RPCIsInWarmup(nullptr)) SetRPCWarmupFinished();

    BOOST_CHECK_NO_THROW(result = tableRPC.execute(request));

    BOOST_CHECK(result.get_str() == "Bitcoin and VeriBlock bloks were added");

    request.strMethod = "getpopdata";
    request.params = UniValue(UniValue::VARR);
    request.fHelp = false;

    request.params.push_back(blockHeight);

    if (RPCIsInWarmup(nullptr)) SetRPCWarmupFinished();

    BOOST_CHECK_NO_THROW(result = tableRPC.execute(request));

    BOOST_CHECK(find_value(result.get_obj(), "last_known_veriblock_blocks").get_array().size() == 16); // number of bustrap blocks that have been set up in .properties file
    BOOST_CHECK(find_value(result.get_obj(), "last_known_bitcoin_blocks").get_array().size() == 16); // number of bustrap blocks that have been set up in .properties file
}
BOOST_AUTO_TEST_SUITE_END()