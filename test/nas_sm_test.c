#include "core_debug.h"
#include "core_pkbuf.h"
#include "core_lib.h"

#include "mme_context.h"
#include "s1ap_message.h"

#include "testutil.h"
#include "tests1ap.h"

static void nas_sm_test1(abts_case *tc, void *data)
{
    status_t rv;
    net_sock_t *sock;
    pkbuf_t *sendbuf;
    pkbuf_t *recvbuf;
    s1ap_message_t message;
    int rc;
    int i;

    c_uint8_t tmp[MAX_SDU_LEN];
    char *_authentication_request = 
        "000b403b00000300 000005c00100009d 000800020001001a 0025240752002008"
        "0c3818183b522614 162c07601d0d10f1 1b89a2a8de8000ad 0ccf7f55e8b20d";
    char *_security_mode_command = 
        "000b402700000300 000005c00100009d 000800020001001a 00111037f933b5d5"
        "00075d010005e060 c04070";
    char *_esm_information_request =
        "000b402000000300 000005c00100009d 000800020001001a 000a092779012320"
        "010221d9";
#if 0
    char *_initial_context_setup_request = "0009"
        "0080e40000060000 0005c00100009d00 0800020001004200 0a183e800000603e"
        "8000000018008092 00003400808c4540 0920000000000000 00000f800a012d2e"
        "00001c957527b1eb ed32020742024906 4000f1105ba0004c 5221c10509ffffff"
        "ff0908696e746572 6e657405012d2d2d 715e060000000004 0427298080211002"
        "0000108106080808 08830604040404c2 230403000004000d 0408080808000d04"
        "04040404500bf600 f110000201040001 fb5312172c594964 0125006b00051800"
        "0c00000049002033 11c603c6a6d67f69 5e5ac02bb75b381b 693c3893a6d932fd"
        "91823544e3e79b";
#endif

    /* eNB connects to MME */
    sock = tests1ap_enb_connect();
    ABTS_PTR_NOTNULL(tc, sock);

    d_log_set_level(D_MSG_TO_STDOUT, D_LOG_LEVEL_FULL);
    /* Send S1-Setup Reqeust */
    rv = tests1ap_build_setup_req(&sendbuf, 0x54f64);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);
    rv = tests1ap_enb_send(sock, sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);

    /* Receive S1-Setup Response */
    recvbuf = pkbuf_alloc(0, MAX_SDU_LEN);
    rc = tests1ap_enb_read(sock, recvbuf);
    ABTS_INT_NEQUAL(tc, 0, rc);
    rv = s1ap_decode_pdu(&message, recvbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);
    s1ap_free_pdu(&message);
    pkbuf_free(recvbuf);

    /* Send Initial-UE Message */
    mme_self()->mme_ue_s1ap_id = 16777372;
    rv = tests1ap_build_initial_ue_msg(&sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);
    rv = tests1ap_enb_send(sock, sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);

    /* Receive Authentication-Request */
    recvbuf = pkbuf_alloc(0, MAX_SDU_LEN);
    rc = tests1ap_enb_read(sock, recvbuf);
    ABTS_INT_NEQUAL(tc, 0, rc);
    recvbuf->len = 63;
    ABTS_TRUE(tc, memcmp(recvbuf->payload, 
        CORE_HEX(_authentication_request, strlen(_authentication_request), tmp),
        recvbuf->len) == 0);
    pkbuf_free(recvbuf);

    /* Send Authentication-Response */
    rv = tests1ap_build_authentication_response(&sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);
    rv = tests1ap_enb_send(sock, sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);

    /* Receive Security-mode-Command */
    recvbuf = pkbuf_alloc(0, MAX_SDU_LEN);
    rc = tests1ap_enb_read(sock, recvbuf);
    recvbuf->len = 43;
    ABTS_TRUE(tc, memcmp(recvbuf->payload, 
        CORE_HEX(_security_mode_command, strlen(_security_mode_command), tmp),
        recvbuf->len) == 0);
    ABTS_INT_NEQUAL(tc, 0, rc);
    pkbuf_free(recvbuf);

    /* Send Security-mode-Complete */
    rv = tests1ap_build_security_mode_complete(&sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);
    rv = tests1ap_enb_send(sock, sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);

    /* Receive ESM Information Request */
    recvbuf = pkbuf_alloc(0, MAX_SDU_LEN);
    rc = tests1ap_enb_read(sock, recvbuf);
    recvbuf->len = 36;
    ABTS_TRUE(tc, memcmp(recvbuf->payload, 
        CORE_HEX(_esm_information_request, strlen(_security_mode_command), tmp),
        recvbuf->len) == 0);
    pkbuf_free(recvbuf);

    /* Send ESM Information Response */
    rv = tests1ap_build_esm_information_response(&sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);
    rv = tests1ap_enb_send(sock, sendbuf);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);

    core_sleep(time_from_msec(300));

    d_log_set_level(D_MSG_TO_STDOUT, D_LOG_LEVEL_ERROR);
    /* eNB disonncect from MME */
    rv = tests1ap_enb_close(sock);
    ABTS_INT_EQUAL(tc, CORE_OK, rv);

    core_sleep(time_from_msec(300));
    d_log_set_level(D_MSG_TO_STDOUT, D_LOG_LEVEL_FULL);
}

abts_suite *test_nas_sm(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, nas_sm_test1, NULL);

    return suite;
}
