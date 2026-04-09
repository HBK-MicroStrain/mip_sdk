
#include <microstrain_test/microstrain_test.hpp>

#include <microstrain/strings.hpp>
#include <mip/mip_packet.hpp>

#include <mip/definitions/commands_base.hpp>


//
// PacketView
//

MICROSTRAIN_TEST_CASE("MIP PacketView", "PacketView ctor can construct an empty packet in a buffer.")
{
    uint8_t buffer[64];
    mip::PacketView packet(buffer, sizeof(buffer), 0x80);

    CHECK(packet.isSane());
    CHECK_EQ(packet.descriptorSet(), 0x80);
    CHECK_EQ(packet.totalLength(), mip::PacketView::LENGTH_MIN);
    CHECK_EQ(packet.buffer().data(), buffer);
    CHECK_EQ(packet.buffer().size(), sizeof(buffer));
}

MICROSTRAIN_TEST_CASE("MIP PacketView", "PacketView can be constructed from a packet buffer.")
{
    // Ping packet
    const uint8_t buffer[] = {0x75, 0x65, 0x01, 0x02, 0x02, 0x01, 0xE0, 0xC6};
    mip::PacketView packet(buffer, sizeof(buffer));

    CHECK(packet.isValid());
    CHECK_EQ(packet.descriptorSet(), 0x01);
    CHECK_EQ(packet.totalLength(), sizeof(buffer));
    CHECK_EQ(packet.buffer().data(), buffer);
    CHECK_EQ(packet.buffer().size(), sizeof(buffer));
}

MICROSTRAIN_TEST_CASE("MIP PacketView", "PacketView can be copy-constructed.")
{
    // Ping packet
    const uint8_t buffer[] = {0x75, 0x65, 0x01, 0x02, 0x02, 0x01, 0xE0, 0xC6};
    mip::PacketView packet0(buffer, sizeof(buffer));

    mip::PacketView packet(packet0);

    CHECK(packet.isValid());
    CHECK_EQ(packet.descriptorSet(), 0x01);
    CHECK_EQ(packet.totalLength(), sizeof(buffer));
    CHECK_EQ(packet.buffer().data(), buffer);
    CHECK_EQ(packet.buffer().size(), sizeof(buffer));
}

MICROSTRAIN_TEST_CASE("MIP PacketView", "dataAt returns the right values.")
{
    // Get comm speed response
    const uint8_t buffer[] = {0x75,0x65,0x01,0x0b,0x04,0xf1,0x09,0x00,0x07,0x89,0x11,0x00,0x01,0xc2,0x00,0x48,0x1c};
    mip::PacketView packet(buffer, sizeof(buffer));

    for(size_t i=0; i<packet.totalLength(); i++)
    {
        INFO("i = " << i);
        CHECK_EQ(packet.dataAt(i), buffer[i]);
    }
}

MICROSTRAIN_TEST_CASE("MIP PacketView", "dataAt returns the right values.")
{
    // Get comm speed response
    const uint8_t buffer[] = {0x75,0x65,0x01,0x0b,0x04,0xf1,0x09,0x00,0x07,0x89,0x11,0x00,0x01,0xc2,0x00,0x48,0x1c};
    mip::PacketView packet(buffer, sizeof(buffer));

    for(size_t i=0; i<packet.payloadLength(); i++)
    {
        INFO("i = " << i);
        CHECK_EQ(packet.payloadAt(i), buffer[4+i]);
    }
}



//
// PacketBuf
//

MICROSTRAIN_TEST_CASE("MIP PacketBuf", "PacketBuf constructs an initialized, empty packet.")
{
    mip::PacketBuf packet(0x80);

    CHECK_EQ(packet.descriptorSet(), 0x80);
    CHECK_EQ(packet.totalLength(), mip::PacketView::LENGTH_MIN);
}

MICROSTRAIN_TEST_CASE("MIP PacketBuf", "PacketBuf can be constructed from an existing packet.")
{
    // Ping packet
    const uint8_t buffer[] = {0x75, 0x65, 0x01, 0x02, 0x02, 0x01, 0xE0, 0xC6};

    mip::PacketBuf packet(mip::PacketView{buffer, sizeof(buffer)});

    CHECK_BUFFER_EQ(packet.data().data(), buffer, sizeof(buffer));
    CHECK_EQ(packet.descriptorSet(), 0x01);
    CHECK_EQ(packet.totalLength(), sizeof(buffer));
    CHECK_NE(packet.buffer().data(), buffer);  // PacketBuf should contain its own buffer
}

MICROSTRAIN_TEST_CASE("MIP PacketBuf", "PacketBuf can be constructed from a raw buffer.")
{
    // Ping packet
    const uint8_t buffer[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    mip::PacketBuf packet(microstrain::ConstU8ArrayView{buffer, sizeof(buffer)});

    CHECK_BUFFER_EQ(packet.data().data(), buffer, sizeof(buffer));
    CHECK_NE(packet.buffer().data(), buffer);  // PacketBuf should contain its own buffer
}

MICROSTRAIN_TEST_CASE("MIP PacketBuf", "PacketBuf can be copy-constructed.")
{
    mip::PacketBuf packet0(0x80);
    packet0.finalize();

    mip::PacketBuf packet(packet0);

    CHECK(packet.isValid());
    CHECK_EQ(packet.descriptorSet(), 0x80);
    CHECK_EQ(packet.totalLength(), mip::PacketView::LENGTH_MIN);
    CHECK_NE(packet.buffer().data(), packet0.buffer().data());  // PacketBuf should contain its own buffer
}

MICROSTRAIN_TEST_CASE("MIP PacketBuf", "PacketBuf be created from a mip command (base comm speed).")
{
    mip::commands_base::CommSpeed commSpeed;
    commSpeed.function = mip::FunctionSelector::WRITE;
    commSpeed.port     = 0x01;
    commSpeed.baud     = 115200;

    mip::PacketBuf packet(commSpeed);

    CHECK(packet.isValid());
    CHECK_EQ(packet.descriptorSet(), mip::commands_base::CommSpeed::DESCRIPTOR_SET);
    CHECK_EQ(packet.payloadLength(), 8);
}

