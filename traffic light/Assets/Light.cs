using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Light : MonoBehaviour {

    public GameObject Red;
    public GameObject Yellow;
    public GameObject GreenL;
    public GameObject GreenR;
    // Update is called once per frame
    void Update () {
        if (Input.GetKeyDown(KeyCode.A))
        {
            Red.SetActive(true);
            Yellow.SetActive(false);
            GreenR.SetActive(false);
            GreenL.SetActive(false);
        }

        if (Input.GetKeyDown(KeyCode.S))
        {
            Red.SetActive(false);
            GreenR.SetActive(false);
            GreenL.SetActive(false);
            Yellow.SetActive(true);
        }

        if (Input.GetKeyDown(KeyCode.D))
        {
            Red.SetActive(false);
            Yellow.SetActive(false);
            GreenR.SetActive(false);
            GreenL.SetActive(true);
        }
        if (Input.GetKeyDown(KeyCode.F))
        {
            Red.SetActive(false);
            Yellow.SetActive(false);
            GreenL.SetActive(false);
            GreenR.SetActive(true);
        }
        if (Input.GetKeyDown(KeyCode.R))
        {
            Red.SetActive(false);
            Yellow.SetActive(false);
            GreenR.SetActive(false);
            GreenL.SetActive(false);
        }
    }
}
